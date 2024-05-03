build:
	@echo "Building the project"
	@mpic++ -o out.out src/*.cpp
	@echo "Build complete"

run: build
	@echo "Running the project"
	@mpiexec -n 4 ./out.out
	@rm out.out
