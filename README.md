# Hybrid B+ Tree and HNSW Index Integration

This repository demonstrates a novel integration of a B+ tree (built from scratch) with the HNSW (Hierarchical Navigable Small World) index. The goal is to maximize recall for queries involving range and equality constraints, areas where the HNSW index alone is less effective. By combining the strengths of both data structures, this project achieves improved performance for a variety of query types.

## Repository Contents

The repository includes:
- **Source Code**: The implementation of the B+ tree, the HNSW index, and the hybrid integration logic.
- **Documentation**: Detailed explanations of the code structure and the hybrid approach are available in the report included in the repository.
- **Examples and Experiments**: Scripts and data samples to demonstrate the effectiveness of the hybrid method.

## File Structure

- `src/`: Contains the source files for the project.
  - `main.cpp`: The main entry point of the application.
  - `utils.cpp`: Utility functions used throughout the project.
  - `HybridDB.cpp`: Implementation of the hybrid B+ tree and HNSW integration.
  - `BPTreeDB.cpp`: Implementation of the custom B+ tree.
  - `HnswDB.cpp`: Implementation of HNSW index integration.
  - `HashHnswDB.cpp`: Implementation of a hashed HNSW approach.
- `data/`: Place your dataset files here.
- `output/`: The directory where output files will be saved.
- `nmslib/` or `hnswlib/`: Copy the respective repository here for the HNSW index implementation.

## Setup Instructions

1. **Add Your Data**: Place your dataset files in a `data` folder at the root of the repository.
2. **Copy HNSW Dependency**: Clone or copy the `nmslib` or `hnswlib` repository at the root of this project.
3. **Compile the Code**: Use the following command to compile the main program along with the required implementations:
   ```bash
   g++ -O3 -o main src/main.cpp src/utils.cpp src/HybridDB.cpp src/BPTreeDB.cpp src/HnswDB.cpp src/HashHnswDB.cpp
