#!/bin/bash
echo "Content-Type: text/plain"
echo ""
echo "CGI Test Script"
echo "================"
echo "Method: $REQUEST_METHOD"
echo "Query: $QUERY_STRING"
echo "User-Agent: $HTTP_USER_AGENT"
echo ""
echo "All environment variables:"
env | sort