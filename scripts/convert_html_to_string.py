import os

# Define the path to the HTML file
html_file_path = "data/access_point.html"

# Define the output file path for the C++ header file
output_header_file_path = "src/html_content.h"

# Check if the HTML file exists
if not os.path.exists(html_file_path):
    print(f"Error: HTML file '{html_file_path}' does not exist.")
    exit(1)

# Read the HTML content
with open(html_file_path, 'r', encoding='utf-8') as html_file:
    html_content = html_file.read()

# Escape double quotes (") for C++ string literals

# Generate C++ code for the header file, using raw string literal syntax
header_content = f"""
#pragma once
const char htmlContent[] PROGMEM = R"rawliteral(
{html_content}
)rawliteral";
"""

# Write the C++ code to the header file
with open(output_header_file_path, 'w', encoding='utf-8') as output_file:
    output_file.write(header_content)

print(f"HTML content has been successfully written to {output_header_file_path}")
