import argparse

def main():
    parser = argparse.ArgumentParser(description='Generate C array from 8-digit hexadecimal numbers.')
    parser.add_argument('input_files', nargs='+', help='Input text files')
    parser.add_argument('-o', '--output', default='output.h', help='Output header file')
    parser.add_argument('-n', '--name', default='data_array', help='Array name')
    args = parser.parse_args()

    numbers = []
    for filename in args.input_files:
        with open(filename, 'r') as f:
            for line_num, line in enumerate(f, 1):
                stripped = line.strip()
                if len(stripped) != 8:
                    print(f"Warning: {filename}:{line_num} - Invalid length '{stripped}' (skipping)")
                    continue
                try:
                    # Convert hex string to integer
                    num = int(stripped, 16)
                    numbers.append(num)
                except ValueError:
                    print(f"Warning: {filename}:{line_num} - Invalid hex value '{stripped}' (skipping)")
                    continue

    with open(args.output, 'w') as f:
        f.write(f"#ifndef {args.name.upper()}_H\n")
        f.write(f"#define {args.name.upper()}_H\n\n")
        f.write(f"unsigned int {args.name}[] = {{\n")
        for i, num in enumerate(numbers):
            comma = ',' if i < len(numbers) - 1 else ''
            f.write(f"    0x{num:08X}{comma}\n")  # Format as 8-digit hex with leading zeros
        f.write("};\n\n")
        f.write(f"#endif // {args.name.upper()}_H\n")

if __name__ == '__main__':
    main()