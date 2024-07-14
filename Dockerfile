# Use an official Python runtime as a parent image
FROM emscripten/emsdk:3.1.63

RUN echo "hello"

# Set the working directory in the container
WORKDIR /

RUN mkdir ./app

# Run the application
# CMD ["python", "-c", "print('Hello, World!')"]
CMD ["/mount/entrypoint.sh"]
