build:
	@echo "Building the project"
	@mpic++ -o out.out src/*.cpp
	@echo "Build complete"

run:build
	@echo "Running the project"
	@mpiexec -n 5 ./out.out $(n) $(file)
	@rm out.out

run_neh50_20:build
	@echo "Running the project"
	@mpiexec -n 5 ./out.out
	@rm out.out
	@echo "Running visualization"
	@python main.py

