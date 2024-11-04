#!/bin/bash

# Parse command line arguments
usage() {
    echo "Usage: $0 --param {color|depth|orb|}"
    exit 1
}

# Initialize param_value with default value
param_value="color"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    key="$1"

    case $key in
        --param)
            param_value="$2"
            shift # past argument
            shift # past value
            ;;
        *)  # unknown option
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

# Construct the command
command="docker exec -it ap4hlc /bin/bash -c \"source train_dagger.bash $param_value\""

# Execute the command
eval "$command"
