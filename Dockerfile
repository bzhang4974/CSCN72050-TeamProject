FROM ubuntu:latest

# Update and install required packages
RUN apt-get update && \
    apt-get install -y \
    g++ \
    cmake \
    libboost-all-dev \
    libasio-dev \
    libssl-dev \
    curl \
    make

# Set working directory
WORKDIR /robot

# Copy all project files into the container
COPY . .

# Create build folder and compile the project
RUN mkdir build && cd build && \
    cmake .. && make

# Expose the port for the web server
EXPOSE 18080

# Run the server
CMD ["./build/RobotController"]