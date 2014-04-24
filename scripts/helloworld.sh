#!/bin/bash
# Tip: Use tee to write to files because > and >> requires an active tty, which won't be available during provisioning.
echo Hello world | sudo tee /helloworld.txt
