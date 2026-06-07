import re
import sys

if len(sys.argv) < 2:
    print("Usage: python3 extract_data.py <mac-nvision.data>")
    sys.exit(1)

input_file = sys.argv[1]
output_file = "memory_image.hex"

section_names = ['.data', '.sdata', '.rodata']
address_word_map = {}

with open(input_file, 'r') as f:
    lines = f.readlines()

current_section = None
for line in lines:
    # Check for section start
    for name in section_names:
        if f"Hex dump of section '{name}':" in line:
            current_section = name
            break
    if current_section:
        # Match address and words
        match = re.match(r'\s*0x([0-9a-fA-F]+)((?:\s+[0-9a-fA-F]{8})+)', line)
        if match:
            addr = int(match.group(1), 16)
            words = match.group(2).split()
            for i, word in enumerate(words):
                address_word_map[addr + 4 * i] = word
        # End section only if line is empty
        if line.strip() == "":
            current_section = None

# If no words found, exit
if not address_word_map:
    print("No words extracted, writing empty memory image.")
    open(output_file, 'w').close()
    sys.exit(0)

# Set start address to 0x00000000
start_addr = 0x00000000
min_addr = min(address_word_map.keys())
max_addr = max(address_word_map.keys())

# Optionally, set end address (default: max_addr)
end_addr = max_addr
# Uncomment and set manually if you want a larger image:
# end_addr = 0x00004FFF


def le_to_be(word):
    # Convert 8 hex digits from little-endian to big-endian
    b = bytes.fromhex(word)
    return b[::-1].hex()

with open(output_file, 'w') as f:
    for addr in range(start_addr, end_addr + 4, 4):
        word = address_word_map.get(addr, '00000000')
        be_word = le_to_be(word)
        f.write(be_word + '\n')

print(f"Extracted {len(address_word_map)} words to {output_file} (from 0x{start_addr:08x} to 0x{end_addr:08x})")
