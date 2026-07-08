#!/bin/bash

# Simple CGI test script
# Save as: test.sh (make executable with chmod +x test.sh)

echo "Content-Type: text/html"
echo ""

cat << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Bash CGI Test</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 40px auto; padding: 20px; background: #0b1020; color: #e5eefc; }
        h1 { color: #60a5fa; }
        .box { background: rgba(18, 26, 49, 0.88); padding: 20px; border-radius: 12px; margin: 15px 0; border: 1px solid rgba(148, 163, 184, 0.16); }
        .green { color: #86efac; }
        .yellow { color: #fde047; }
        .red { color: #fca5a5; }
        pre { background: rgba(7, 12, 26, 0.7); padding: 15px; border-radius: 8px; overflow: auto; }
        table { width: 100%; border-collapse: collapse; }
        td { padding: 6px 10px; border-bottom: 1px solid rgba(148, 163, 184, 0.1); }
        .key { color: #a9b8d3; font-weight: bold; }
    </style>
</head>
<body>
    <h1>🚀 Bash CGI Test Script</h1>
    <p>This script is executed by the web server via CGI.</p>

    <div class="box">
        <h2>📋 Request Information</h2>
        <table>
            <tr><td class="key">Request Method:</td><td>${REQUEST_METHOD:-Not set}</td></tr>
            <tr><td class="key">Script Name:</td><td>${SCRIPT_NAME:-Not set}</td></tr>
            <tr><td class="key">Server Protocol:</td><td>${SERVER_PROTOCOL:-Not set}</td></tr>
            <tr><td class="key">Remote Address:</td><td>${REMOTE_ADDR:-Not set}</td></tr>
            <tr><td class="key">User Agent:</td><td>${HTTP_USER_AGENT:-Not set}</td></tr>
        </table>
    </div>

    <div class="box">
        <h2>📝 Query String</h2>
        <p>${QUERY_STRING:-<span class="yellow">(empty)</span>}</p>
EOF

# Parse and display query parameters
if [ -n "$QUERY_STRING" ]; then
    echo "<p><strong>Parsed parameters:</strong></p><ul>"
    IFS='&' read -ra pairs <<< "$QUERY_STRING"
    for pair in "${pairs[@]}"; do
        key=$(echo "$pair" | cut -d'=' -f1)
        value=$(echo "$pair" | cut -d'=' -f2-)
        echo "<li><span class='key'>$key</span> = <span class='green'>$value</span></li>"
    done
    echo "</ul>"
fi

cat << EOF
    </div>

    <div class="box">
        <h2>📥 POST Data</h2>
        <pre>$(cat - 2>/dev/null || echo "(no POST data)")</pre>
    </div>

    <div class="box">
        <h2>🔧 Environment Variables</h2>
        <table>
            <tr><td class="key">SERVER_SOFTWARE:</td><td>${SERVER_SOFTWARE:-Not set}</td></tr>
            <tr><td class="key">SERVER_NAME:</td><td>${SERVER_NAME:-Not set}</td></tr>
            <tr><td class="key">GATEWAY_INTERFACE:</td><td>${GATEWAY_INTERFACE:-Not set}</td></tr>
            <tr><td class="key">CONTENT_LENGTH:</td><td>${CONTENT_LENGTH:-0}</td></tr>
            <tr><td class="key">CONTENT_TYPE:</td><td>${CONTENT_TYPE:-Not set}</td></tr>
EOF

# Show additional HTTP headers if they exist
for var in HTTP_ACCEPT HTTP_REFERER HTTP_COOKIE; do
    if [ -n "${!var}" ]; then
        echo "<tr><td class='key'>${var//_/-}:</td><td>${!var}</td></tr>"
    fi
done

cat << EOF
        </table>
    </div>

    <div class="box">
        <h2>⏰ Server Info</h2>
        <table>
            <tr><td class="key">Current Date:</td><td>$(date)</td></tr>
            <tr><td class="key">Server Timezone:</td><td>$(date +%Z)</td></tr>
            <tr><td class="key">Hostname:</td><td>$(hostname 2>/dev/null || echo "unknown")</td></tr>
        </table>
    </div>

    <div class="box" style="border-color: rgba(104, 211, 145, 0.3);">
        <h2 class="green">✅ CGI Execution Successful</h2>
        <p>This script was executed at <strong>$(date)</strong></p>
        <p class="yellow">Status: 200 OK</p>
    </div>
</body>
</html>
EOF