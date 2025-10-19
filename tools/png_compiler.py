import os
import argparse
import re
from datetime import datetime, timezone

def convert_png_to_header(input_path, output_path, array_name, namespace):
    # Read the PNG file as binary data
    with open(input_path, 'rb') as f:
        data = f.read()

    # If array name is not provided, derive it from the input file name (without extension)
    if array_name is None:
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        array_name = re.sub(r'\W|^(?=\d)', '_', base_name)

    # Ensure the array name ends with "_png"
    if not array_name.endswith("_png"):
        array_name = array_name + "_png"

    # Get the current timezone-aware UTC date and time for the header comment
    generation_time = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S %Z")
    header_comment = f"// This file was generated on {generation_time}\n"

    # Format the binary data as a comma-separated list of hex numbers
    array_size = len(data)
    array_elements = ', '.join('0x{:02X}'.format(b) for b in data)

    # Default header content with the generated array
    header_content = f"""{header_comment}#pragma once

#include <array>

namespace {namespace} {{
    constexpr std::array<unsigned char, {array_size}> {array_name} = {{
        {array_elements}
    }};
}}
"""

    # Write the generated header to the specified output file
    with open(output_path, 'w') as f:
        f.write(header_content)

    print(f"Header file generated at: {output_path}")

def main():
    parser = argparse.ArgumentParser(
        description="Convert a PNG image to a C++ header file containing a std::array for SFML."
    )
    parser.add_argument("input", help="Path to the PNG image file")
    parser.add_argument(
        "-o", "--output",
        help="Output header file location (defaults to './image_data.hpp')",
        default="image_data.hpp"
    )
    parser.add_argument(
        "-a", "--array-name",
        help="Name of the generated array (default is derived from input file name)",
        default=None
    )
    parser.add_argument(
        "-n", "--namespace",
        help="Namespace for the generated array (default is 'generated')",
        default="generated"
    )
    args = parser.parse_args()

    convert_png_to_header(args.input, args.output, args.array_name, args.namespace)

if __name__ == "__main__":
    main()
