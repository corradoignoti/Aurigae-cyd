#!/usr/bin/env python3
"""
Script to extract non-ASCII characters from weather.ino for LVGL font generation.
This helps identify which extended characters are needed for multilingual support.
"""

import sys
import re
from collections import Counter

def extract_non_ascii_chars(file_path):
    """Extract and analyze non-ASCII characters from the source file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        return None, None, None
    except UnicodeDecodeError:
        print(f"Error: Could not decode file '{file_path}' as UTF-8.")
        return None, None, None
    
    # Find all non-ASCII characters (code point > 127)
    non_ascii_chars = set()
    char_counter = Counter()
    
    for char in content:
        if ord(char) > 127:  # Non-ASCII characters
            non_ascii_chars.add(char)
            char_counter[char] += 1
    
    return non_ascii_chars, char_counter, content

def print_character_analysis(non_ascii_chars, char_counter):
    """Print detailed analysis of the non-ASCII characters found."""
    if not non_ascii_chars:
        print("No non-ASCII characters found in the file.")
        return
    
    print("="*60)
    print("NON-ASCII CHARACTER ANALYSIS FOR LVGL FONT")
    print("="*60)
    
    print(f"\nTotal unique non-ASCII characters found: {len(non_ascii_chars)}")
    
    # Sort characters by Unicode code point
    sorted_chars = sorted(non_ascii_chars, key=ord)
    
    print("\n1. COMPLETE CHARACTER LIST:")
    print("-" * 30)
    char_list = ''.join(sorted_chars)
    print(f"Characters: {char_list}")
    
    print("\n2. DETAILED CHARACTER INFO:")
    print("-" * 30)
    print(f"{'Char':<4} {'Unicode':<8} {'Name':<25} {'Count':<6}")
    print("-" * 45)
    
    for char in sorted_chars:
        unicode_point = f"U+{ord(char):04X}"
        try:
            import unicodedata
            name = unicodedata.name(char)
        except ValueError:
            name = "UNKNOWN"
        count = char_counter[char]
        print(f"'{char}'  {unicode_point:<8} {name:<25} {count:<6}")
    
    print("\n3. CHARACTERS BY LANGUAGE:")
    print("-" * 30)
    
    # Categorize by common language groups
    spanish_chars = set("ñáéíóúüÑÁÉÍÓÚÜ¿¡")
    german_chars = set("äöüßÄÖÜ")
    french_chars = set("àâäçéèêëïîôùûüÿæœÀÂÄÇÉÈÊËÏÎÔÙÛÜŸÆŒ")
    
    spanish_found = non_ascii_chars & spanish_chars
    german_found = non_ascii_chars & german_chars
    french_found = non_ascii_chars & french_chars
    other_found = non_ascii_chars - spanish_chars - german_chars - french_chars
    
    if spanish_found:
        print(f"Spanish: {''.join(sorted(spanish_found))}")
    if german_found:
        print(f"German:  {''.join(sorted(german_found))}")
    if french_found:
        print(f"French:  {''.join(sorted(french_found))}")
    if other_found:
        print(f"Other:   {''.join(sorted(other_found))}")
    
    print("\n4. LVGL FONT CONVERTER INPUT:")
    print("-" * 30)
    print("Copy this character range for LVGL font converter:")
    print(f"'{char_list}'")
    
    print("\n5. UNICODE RANGES:")
    print("-" * 30)
    ranges = []
    sorted_codes = sorted([ord(char) for char in non_ascii_chars])
    
    if sorted_codes:
        start = sorted_codes[0]
        end = sorted_codes[0]
        
        for code in sorted_codes[1:]:
            if code == end + 1:
                end = code
            else:
                ranges.append((start, end))
                start = end = code
        ranges.append((start, end))
    
    for start, end in ranges:
        if start == end:
            print(f"U+{start:04X}")
        else:
            print(f"U+{start:04X}-U+{end:04X}")
    
    print("\n6. CHARACTER FREQUENCY:")
    print("-" * 30)
    most_common = char_counter.most_common(10)
    for char, count in most_common:
        print(f"'{char}' appears {count} times")
    
    print("\n" + "="*60)

def extract_string_literals(content):
    """Extract string literals to focus analysis on user-visible text."""
    # Pattern to match string literals in C/C++
    string_pattern = r'"([^"\\]|\\.)*"'
    matches = re.findall(string_pattern, content)
    
    print("\n7. STRING LITERALS ANALYSIS:")
    print("-" * 30)
    
    string_non_ascii = set()
    for match in matches:
        # Remove surrounding quotes and process escape sequences
        string_content = match[1:-1] if match.startswith('"') and match.endswith('"') else match
        for char in string_content:
            if ord(char) > 127:
                string_non_ascii.add(char)
    
    if string_non_ascii:
        sorted_string_chars = sorted(string_non_ascii, key=ord)
        print(f"Non-ASCII chars in strings: {''.join(sorted_string_chars)}")
        print("These are the characters most likely to be displayed to users.")
    else:
        print("No non-ASCII characters found in string literals.")

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 extract_unicode_chars.py <path_to_weather.ino>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    
    non_ascii_chars, char_counter, content = extract_non_ascii_chars(file_path)
    
    if non_ascii_chars is None:
        sys.exit(1)
    
    print_character_analysis(non_ascii_chars, char_counter)
    
    if content:
        extract_string_literals(content)
    
    print(f"\n✅ Analysis complete! Found {len(non_ascii_chars)} unique non-ASCII characters.")
    print("Use the character list above with LVGL's font converter tool.")

if __name__ == "__main__":
    main()