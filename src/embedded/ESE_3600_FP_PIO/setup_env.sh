#!/bin/bash

# Make this script executable if it isn't already
if [ ! -x "$0" ]; then
    chmod +x "$0"
fi

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Setting up Python virtual environment...${NC}"

# Delete existing .venv if it exists
if [ -d ".venv" ]; then
    echo -e "${RED}Removing existing virtual environment...${NC}"
    rm -rf .venv
fi

# Create new virtual environment
echo -e "${BLUE}Creating new virtual environment...${NC}"
python3 -m venv .venv

# Activate virtual environment
echo -e "${BLUE}Activating virtual environment...${NC}"
source .venv/bin/activate

# Upgrade pip
echo -e "${BLUE}Upgrading pip...${NC}"
pip install --upgrade pip

# Install requirements
echo -e "${BLUE}Installing requirements...${NC}"
pip install -r requirements.txt

echo -e "${GREEN}Setup complete!${NC}"
echo -e "${GREEN}To activate the virtual environment, run:${NC}"
echo -e "${BLUE}source .venv/bin/activate${NC}" 