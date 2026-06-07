import re
import sys

with open(sys.argv[1], 'r') as f:
    disasm = f.readlines()

hex_instructions = []
for line in disasm:
    match = re.match(r'\s*[0-9a-f]+:\s+([0-9a-f]+)\s+', line)
    if match:
        hex_instructions.append(match.group(1))

with open('instructions.hex', 'w') as f:
    f.write('\n'.join(hex_instructions))