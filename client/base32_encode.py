import base64
import sys


def base32_encode(input_string):
    """Encode a string using base32 encoding"""
    if isinstance(input_string, str):
        input_bytes = input_string.encode("utf-8")
    else:
        input_bytes = input_string

    encoded = base64.b32encode(input_bytes)
    return encoded.decode("utf-8")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python base32_encode.py <string_to_encode>")
        sys.exit(1)

    input_string = sys.argv[1]
    encoded = base32_encode(input_string)
    print(encoded)
