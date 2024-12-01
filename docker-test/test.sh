#!/bin/bash

# Array of test endpoints
endpoints=(
    "/cgi-bin"
    "/"
    "/uploads"
    "/media/meme.jpg"
    "/redirect"
    "/delete"
)

# Test each endpoint against both servers
for endpoint in "${endpoints[@]}"; do
    echo "Testing $endpoint"
    echo "Nginx response:"
    curl -v "http://localhost:8080${endpoint}"
    echo -e "\nWebserver response:"
    curl -v "http://localhost:8083${endpoint}"
    echo -e "\n-------------------\n"
done 