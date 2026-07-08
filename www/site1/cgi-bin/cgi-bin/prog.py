#!/usr/bin/env python3
import os, sys
print("Content-Type: text/plain\n")
print("CGI Test (Python)")
print("=" * 30)
print(f"Method: {os.environ.get('REQUEST_METHOD', 'N/A')}")
print(f"Query: {os.environ.get('QUERY_STRING', 'N/A')}")
print(f"UA: {os.environ.get('HTTP_USER_AGENT', 'N/A')}")
print("\nAll env vars:\n" + "\n".join(f"{k}: {v}" for k, v in sorted(os.environ.items())))