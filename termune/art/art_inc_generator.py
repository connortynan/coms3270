import sys
import os


def main():
    infile = sys.argv[1]
    outfile = sys.argv[2]
    compat_outfile = outfile + ".compat"

    with open(infile, "r", encoding="utf-8") as f:
        content = f.read()

    parts = content.splitlines()
    blocks = []
    current_block = []

    for line in parts:
        if line.strip() == "*":
            if current_block:
                blocks.append(current_block)
                current_block = []
        else:
            current_block.append(line)
    if current_block:
        blocks.append(current_block)

    # Generate .inc file
    with open(outfile, "w", encoding="utf-8") as out:
        out.write(
            "/** Auto-generated ASCII art include file\n"
            f" * Num Frames: {len(blocks)}, Dimensions: ({len(blocks[0][0])}, {len(blocks[0])}) characters\n"
            " * Usage: \n"
            " *     const wchar_t *art[NUM_FRAMES][ART_HEIGHT] = {\n"
            f' * #include "{os.path.basename(outfile)}"\n'
            " *     };\n"
            " */\n\n"
        )
        for block in blocks:
            out.write("{\n")
            for line in block:
                escaped = line.replace("\\", "\\\\").replace('"', '\\"')
                out.write(f'L"{escaped}",\n')
            out.write("},\n")

    # Generate .compat file
    with open(compat_outfile, "w", encoding="utf-8") as out:
        out.write(
            "/** Auto-generated ASCII art (compat mode)\n"
            " * All characters are either space or '#'.\n"
            " * Usage:\n"
            " *     const char *art[NUM_FRAMES][ART_HEIGHT] = {\n"
            f' * #include "{os.path.basename(compat_outfile)}"\n'
            " *     };\n"
            " */\n\n"
        )
        for block in blocks:
            out.write("{\n")
            for line in block:
                filtered = "".join(" " if c == " " else "#" for c in line)
                escaped = filtered.replace("\\", "\\\\").replace('"', '\\"')
                out.write(f'"{escaped}",\n')
            out.write("},\n")


if __name__ == "__main__":
    main()
