import socket
import os
import hashlib
from dotenv import load_dotenv

import hmac
import hashlib
from datetime import datetime, timezone
import base64
import struct

TOTP_TIME_STEP = 30
DIGITS = 6

def generate_totp(secret, current_time: int):
    key_decoded = None
    try:
        key_decoded = base64.b32decode(secret.upper() + "=" * (-len(secret) % 8))
    except Exception:
        pass

    if key_decoded is None:
        raise ValueError("TOTP secret key not initialized. Call totp_init first.")

    t = current_time // TOTP_TIME_STEP
    t_bytes = struct.pack(">Q", t)
    assert len(t_bytes) == 8, "Time step must be 8 bytes long"
    hmac_hash = hmac.new(key_decoded, t_bytes, hashlib.sha256).digest()
    assert len(hmac_hash) == 32, "HMAC-SHA256 must produce a 32-byte hash"
    offset = hmac_hash[-1] & 0x0F
    truncated_hash = hmac_hash[offset : offset + 4]
    otp_value = struct.unpack(">I", truncated_hash)[0] & 0x7FFFFFFF
    # 桁数に合わせてOTPを生成
    power_of_10 = 10**DIGITS
    otp = str(otp_value % power_of_10).zfill(DIGITS)
    return otp


def send_message(addr, port, message):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (addr, port)
    try:
        client_socket.connect(server_address)
        print(f"Connected to {server_address[0]}:{server_address[1]}")
        client_socket.sendall(message.encode("utf-8"))
        print(f"Sent: {message}")
        response = client_socket.recv(1024)
        print(f"Received: {response.decode('utf-8')}")

    except ConnectionRefusedError:
        print("Connection failed. Make sure the server is running.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        client_socket.close()
        print("Connection closed")


def main():
    load_dotenv()
    HOST = os.getenv("HOST")
    PORT = int(os.getenv("PORT"))
    SECRET_KEY = os.getenv("SECRET_KEY")

    print(f"Using HOST: {HOST}, PORT: {PORT}, SECRET_KEY: {SECRET_KEY}")

    if not all([HOST, PORT, SECRET_KEY]):
        print("エラー: .envファイルにHOST, PORT, SECRET_KEYを設定してください。")
        return

    current_time_stamp = int(datetime.now(timezone.utc).timestamp())
    print(
        f"Now: {datetime.fromtimestamp(current_time_stamp, timezone.utc).strftime('%Y-%m-%d %H:%M:%S')}"
    )
    otp = generate_totp(SECRET_KEY, current_time_stamp)
    print(f"Generated TOTP: {otp}")
    # send_message(HOST, PORT, otp)


if __name__ == "__main__":
    main()
