'''
---------------------------------------------------------------------------
    Copyright (c) 2026 Barter For Things Ltd. All Rights Reserved.

    Author: Sayed Tanimun Hasan
    Position: Electronics Design Engineer
    Company: Barter For Things Ltd.

    Software: MSP430FR5043 UART Firmware Flasher
    Version: 1.0
    Created: March 2026

    Description:
    This software is designed to program and update firmware on the
    MSP430FR5043 microcontroller using the UART Bootloader (BSL) protocol.
    The tool transfers firmware images to the device via serial
    communication and supports automated flashing and verification.
    Features:
    - UART firmware transfer
    - BSL password authentication
    - Automated firmware upload
    - Device reset after flashing

    Confidentiality:
    This software contains proprietary information of
    Barter For Things Ltd. Unauthorized copying, distribution,
    modification, or use of this software without written permission
    from Barter For Things Ltd is strictly prohibited.

    © 2026 Barter For Things Ltd.
---------------------------------------------------------------------------
'''

#!/usr/bin/env python3
# MSP430FR5xx/FR6xx UART-BSL flasher (v3 – fixed RX_DATA_BLOCK + response checking)
# Flow: unlock -> (optional mass erase) -> write all TI-TXT segments ->
#       repair/program 0xFE00–0xFFFF vectors (-> entry) -> (optional) LOAD_PC entry
# Notes:
# - UART 8E1; no DTR/RTS toggles (kept deasserted).
# - Chunk auto-shrinks on PI 0x54 (packet too large). Retries/backoff on 0x51.
# - Entry priority: --run-addr (if given) -> @FFFE in TXT -> first FRAM code segment (>=0x4400).
# - If you omit --run, just power-cycle; the repaired vectors ensure cold boot works.

import argparse, re, time
from pathlib import Path
import serial

# ---- BSL commands / statuses ----
CMD_RX_PASSWORD    = 0x11
CMD_RX_DATA_BLOCK  = 0x10
CMD_MASS_ERASE     = 0x15
CMD_LOAD_PC        = 0x17
CMD_CHANGE_BAUD    = 0x52
CMD_MESSAGE        = 0x3B   # BSL core response command byte
ACK_OK             = 0x00

BAUD_RATE_MAP = {
    9600:   0x02,
    19200:  0x03,
    38400:  0x04,
    57600:  0x05,
    115200: 0x06,
}

PI_ERRORS = {
    0x51: "PI header/format error",
    0x52: "PI CRC error",
    0x53: "PI packet size zero",
    0x54: "PI packet too large",
    0x55: "PI unknown command",
    0x56: "PI unknown baud rate",
}

BSL_STATUS = {
    0x00: "Operation successful",
    0x01: "Flash write check failed",
    0x02: "Flash fail bit set",
    0x03: "Voltage change during program",
    0x04: "BSL locked",
    0x05: "BSL password error",
    0x06: "Byte write forbidden",
    0x07: "Unknown command",
    0x08: "Packet length exceeds buffer size",
}

def crc16_ccitt(data: bytes, init=0xFFFF) -> int:
    crc = init
    for b in data:
        crc ^= (b << 8) & 0xFFFF
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if (crc & 0x8000) else (crc << 1) & 0xFFFF
    return crc

# ---- TI-TXT helpers ----
def parse_ti_txt(path: Path):
    segs, addr, buf = [], None, bytearray()
    for s in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        s = s.strip()
        if not s: continue
        if s[0] == '@':
            if addr is not None and buf:
                segs.append((addr, bytes(buf))); buf = bytearray()
            addr = int(s[1:], 16)
        elif s[0] in 'qQ':
            break
        else:
            for tok in s.split():
                buf.append(int(tok, 16))
    if addr is not None and buf:
        segs.append((addr, bytes(buf)))
    segs.sort(key=lambda x: x[0])
    return segs

def get_reset_vector_from_segments(segs):
    for base, data in segs:
        if base <= 0xFFFE <= base + len(data) - 2:
            off = 0xFFFE - base
            lo, hi = data[off], data[off + 1]
            if (lo, hi) == (0xFF, 0xFF):
                return None
            return lo | (hi << 8)
    return None

def first_code_entry(segs):
    cands = [b for (b, _) in segs if b >= 0x4400]
    return min(cands) if cands else None

# ---- UART PI transport ----
class FRxxBSL:
    def __init__(self, port, baud=9600, pre_ms=8, post_ms=12, byte_gap_us=0, timeout=2.5):
        self.pre  = pre_ms / 1000.0
        self.post = post_ms / 1000.0
        self.gap  = max(0, byte_gap_us) / 1_000_000.0
        self.ser  = serial.Serial(port=port, baudrate=baud, bytesize=8,
                                  parity=serial.PARITY_EVEN, stopbits=1,
                                  timeout=timeout, write_timeout=timeout)
        # keep control lines deasserted
        try:
            self.ser.dtr = False
            self.ser.rts = False
        except Exception:
            pass

    def _write_streamed(self, data: bytes):
        if self.gap == 0:
            self.ser.write(data); self.ser.flush(); return
        for x in data:
            self.ser.write(bytes([x])); self.ser.flush(); time.sleep(self.gap)

    def _send_core(self, core: bytes):
        if self.pre: time.sleep(self.pre)
        n    = len(core)
        hdr  = bytes([0x80, n & 0xFF, (n >> 8) & 0xFF])
        csum = crc16_ccitt(core)
        tail = bytes([csum & 0xFF, (csum >> 8) & 0xFF])
        try: self.ser.reset_input_buffer()
        except Exception: pass
        self._write_streamed(hdr + core + tail)

    def _expect(self, timeout=None):
        old = self.ser.timeout
        if timeout is not None: self.ser.timeout = timeout
        try:
            b = self.ser.read(1)
            if not b: raise TimeoutError("No response from target")
            v = b[0]
            if v == ACK_OK:
                if self.post: time.sleep(self.post)
                return None
            if v == 0x80:
                nl = self.ser.read(1); nh = self.ser.read(1)
                if len(nl)!=1 or len(nh)!=1: raise TimeoutError("Short len")
                n = nl[0] | (nh[0] << 8)
                core = self.ser.read(n)
                if len(core)!=n: raise TimeoutError("Short core")
                ckl = self.ser.read(1); ckh = self.ser.read(1)
                if len(ckl)!=1 or len(ckh)!=1: raise TimeoutError("Short CRC")
                if (ckl[0] | (ckh[0] << 8)) != crc16_ccitt(core):
                    raise RuntimeError("Response CRC mismatch")
                if self.post: time.sleep(self.post)
                # Check BSL status if this is a core message response
                if len(core) >= 2 and core[0] == CMD_MESSAGE and core[1] != 0x00:
                    status = core[1]
                    desc = BSL_STATUS.get(status, "Unknown error")
                    raise RuntimeError(f"BSL status 0x{status:02X} – {desc}")
                return core
            if v in PI_ERRORS:
                raise RuntimeError(f"PI error 0x{v:02X}: {PI_ERRORS[v]}")
            raise RuntimeError(f"Unexpected leading byte 0x{v:02X}")
        finally:
            if timeout is not None: self.ser.timeout = old

    def send_password(self, pw32: bytes):
        if len(pw32) != 32:
            raise ValueError("Password must be 32 bytes")
        self._send_core(bytes([CMD_RX_PASSWORD]) + pw32)
        self._expect()

    def mass_erase(self):
        self._send_core(bytes([CMD_MASS_ERASE]))
        self._expect(timeout=10)  # mass erase can take several seconds

    def rx_data(self, addr: int, block: bytes):
        if (addr & 1) or (len(block) & 1):
            raise ValueError("addr/len must be even")
        al, am, ah = addr & 0xFF, (addr >> 8) & 0xFF, (addr >> 16) & 0xFF
        # FIX: no length field in core – per SLAU550 the format is CMD AL AM AH D1..Dn
        self._send_core(bytes([CMD_RX_DATA_BLOCK, al, am, ah]) + block)
        self._expect()

    def change_baud_rate(self, new_baud: int):
        code = BAUD_RATE_MAP.get(new_baud)
        if code is None:
            raise ValueError(f"Unsupported baud rate {new_baud}. Use: {list(BAUD_RATE_MAP.keys())}")
        self._send_core(bytes([CMD_CHANGE_BAUD, code]))
        self._expect()
        # Switch host serial port to new baud rate
        self.ser.baudrate = new_baud
        time.sleep(0.05)  # let BSL settle at new baud
        print(f"Baud rate changed to {new_baud}")

    def load_pc(self, addr: int):
        al, am, ah = addr & 0xFF, (addr >> 8) & 0xFF, (addr >> 16) & 0xFF
        try: self.ser.reset_input_buffer()
        except Exception: pass
        self._send_core(bytes([CMD_LOAD_PC, al, am, ah]))
        # no expect(): ROM typically jumps immediately

    def close(self):
        try: self.ser.close()
        except Exception: pass

# ---- CLI / main ----
def read_password_file(p: Path) -> bytes:
    toks = [int(t, 16) for t in re.findall(r"[0-9A-Fa-f]{2}", p.read_text())]
    if len(toks) != 32:
        raise SystemExit("Password file must contain exactly 32 hex bytes.")
    return bytes(toks)

def main():
    ap = argparse.ArgumentParser(description="MSP430 FR5xx/FR6xx UART-BSL flasher with vector repair")
    ap.add_argument("--port", required=True)
    ap.add_argument("--baud", type=int, default=9600,
                    help="Transfer baud rate (BSL always starts at 9600, auto-switches if higher)")
    ap.add_argument("--firmware", required=True)
    ap.add_argument("--password-file")
    ap.add_argument("--no-password", action="store_true")
    ap.add_argument("--erase", action="store_true", help="Mass erase before programming")
    ap.add_argument("--progress", action="store_true")
    ap.add_argument("--run", action="store_true", help="LOAD_PC after programming")
    ap.add_argument("--run-addr", type=lambda x: int(x, 16), help="Override entry address, e.g. 0x6532")
    ap.add_argument("--chunk", type=int, default=240)
    ap.add_argument("--min-chunk", type=int, default=128)
    ap.add_argument("--post-ms", type=int, default=12)
    ap.add_argument("--first-wait-ms", type=int, default=150,
                    help="Quiet time after unlock/erase before first data packet")
    args = ap.parse_args()

    segs = parse_ti_txt(Path(args.firmware))
    if not segs:
        raise SystemExit("No segments parsed from TI-TXT.")

    file_entry = get_reset_vector_from_segments(segs)
    entry = args.run_addr or file_entry or first_code_entry(segs)
    if args.run and entry is None:
        raise SystemExit("Cannot infer entry; pass --run-addr 0xHHHH or include @FFFE in TI-TXT.")

    total = sum(len(d) for _, d in segs)
    print(f"Segments: {', '.join(f'@{b:04X}({len(d)}B)' for b,d in segs)}")
    print(f"TOTAL payload = {total} bytes")
    print(f"Entry point = 0x{entry:04X} (source: {'--run-addr' if args.run_addr else '@FFFE in TXT' if file_entry else 'first code seg'})")

    # BSL always starts at 9600; we switch after unlock if user wants faster
    init_baud = 9600
    target_baud = args.baud

    bsl = FRxxBSL(args.port, init_baud, post_ms=args.post_ms)
    try:
        # Unlock
        if not args.no_password:
            if args.password_file:
                pw = read_password_file(Path(args.password_file))
                print("Using password from file …")
            else:
                pw = bytes([0xFF] * 32)
                print("Using default FF×32 password …")
            bsl.send_password(pw)
            print("Unlocked.")

        # Optional erase (at 9600 – before baud change, since erase may reset BSL state)
        if args.erase:
            print("Mass erase …")
            bsl.mass_erase()
            time.sleep(0.10)

        # Switch baud rate AFTER erase, so it sticks for the data transfer
        if target_baud != init_baud:
            bsl.change_baud_rate(target_baud)

        # Give target some quiet time before first data frame
        time.sleep(args.first_wait_ms / 1000.0)

        # Stream all segments with error handling
        current_chunk = (args.chunk & ~1) or 240
        min_chunk     = max(32, args.min_chunk) & ~1
        sent = 0
        t0 = time.perf_counter()

        for base, data in segs:
            off, L = 0, len(data)
            while off < L:
                n = min(current_chunk, L - off) & ~1
                if n <= 0: n = 2
                addr = base + off
                blk_plain = data[off:off + n]
                if (len(blk_plain) & 1):
                    blk = bytes(bytearray(blk_plain) + b'\xFF'); adv = len(blk_plain)
                else:
                    blk = blk_plain; adv = n
                if args.progress:
                    head = " ".join(f"{b:02X}" for b in blk[:8])
                    print(f"W 0x{addr:06X} +{len(blk):<3} [{head}{' …' if len(blk) > 8 else ''}] (chunk={current_chunk})")
                tries = 0
                while True:
                    try:
                        bsl.rx_data(addr, blk)
                        sent += adv; off += adv
                        break
                    except RuntimeError as e:
                        s = str(e)
                        if "0x54" in s:
                            new_chunk = max(min_chunk, current_chunk - 16) & ~1
                            if new_chunk < current_chunk:
                                print(f"PI 0x54: shrink chunk {current_chunk}→{new_chunk}")
                                current_chunk = new_chunk
                                # rebuild blk for new n
                                n = min(current_chunk, L - off) & ~1
                                if n <= 0: n = 2
                                blk_plain = data[off:off + n]
                                if (len(blk_plain) & 1):
                                    blk = bytes(bytearray(blk_plain) + b'\xFF'); adv = len(blk_plain)
                                else:
                                    blk = blk_plain; adv = n
                                time.sleep(0.01)
                                continue
                            raise
                        if "0x51" in s:
                            tries += 1
                            if tries <= 6:
                                backoff = 0.01 * tries
                                print(f"PI 0x51 transient; retry {tries} after {int(backoff * 1000)} ms …")
                                time.sleep(backoff)
                                continue
                            raise RuntimeError("Repeated PI 0x51; aborting this block.")
                        raise

        dt = max(time.perf_counter() - t0, 1e-6)
        print(f"Sent {sent} bytes in {dt:.2f}s ({(sent / 1024.0) / dt:.2f} KB/s)")

        # ---- Repair/synthesize top segment 0xFE00–0xFFFF (vectors) ----
        top_base, top_size = 0xFE00, 0x200
        top = bytearray([0xFF] * top_size)

        # Merge anything the TXT already had in the top segment
        for base, data in segs:
            s = max(base, top_base)
            e = min(base + len(data), top_base + top_size)
            if e > s:
                top[s - top_base : e - top_base] = data[s - base : s - base + (e - s)]

        # Preserve TLV/signatures (0xFF80..0xFF8F) as merged.
        # Fill every vector 0xFF90..0xFFFE to entry if still erased.
        if entry is not None:
            for vec in range(0xFF90, 0x10000, 2):
                off = vec - top_base
                if top[off] == 0xFF and top[off + 1] == 0xFF:
                    top[off]     = entry & 0xFF
                    top[off + 1] = (entry >> 8) & 0xFF
            # Ensure reset vector matches entry even if TXT had a different value
            off = 0xFFFE - top_base
            top[off]     = entry & 0xFF
            top[off + 1] = (entry >> 8) & 0xFF
            print(f"Vector repair: set all vectors to 0x{entry:04X}; @FFFE = {top[off]:02X} {top[off+1]:02X}")

            # Program entire top segment
            print("Writing repaired top segment 0xFE00..0xFFFF …")
            addr = top_base
            offb = 0
            while offb < top_size:
                n = min(current_chunk, top_size - offb) & ~1
                if n <= 0: n = 2
                blk = bytes(top[offb : offb + n])
                bsl.rx_data(addr + offb, blk)
                offb += n

        # ---- Optional immediate run ----
        if args.run and entry is not None:
            print(f"LOAD_PC -> 0x{entry:04X}")
            bsl.load_pc(entry)

        print("Done.")
    finally:
        bsl.close()

if __name__ == "__main__":
    main()
# python final_flasher.py --port COM12 --baud 115200 --firmware firmware.txt --password-file password.txt --erase --progress --run --chunk 240 --post-ms 12 --first-wait-ms 150
