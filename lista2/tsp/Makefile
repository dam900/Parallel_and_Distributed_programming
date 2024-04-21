build:
	@echo "Building the project"
	@mpic++ -o out.out tsp_problem.cpp include/pugixml.cpp 
	@echo "Build complete"

run:build
	@echo "Running the project"
	@mpiexec -n 5 ./out.out $(n) $(file)
	@rm out.out

run_burma:build
	@echo "Running the project"
	@mpiexec -n 5 ./out.out 14 ./data/burma14.xml
	@rm out.out