#!/bin/bash

gcc database/program_database.c -o database/program_database
gcc client/program_client.c -o client/program_client

echo "Compilation completed."