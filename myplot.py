import pandas as pd
import matplotlib.pyplot as plt
import sys

def myplot(file_path, output_path=""):
    # Load the CSV from the file
    df = pd.read_csv(file_path)

    # Plot the watts for each GPUId
    plt.figure(figsize=(10, 6))
    for gpu_id in df['GPUId'].unique():
        subset = df[df['GPUId'] == gpu_id].reset_index(drop=True)
        print (f"GPU{gpu_id} samples: {len(subset)}")
        plt.plot(subset.index, subset['Watts'], marker='o', linestyle='-', label=f'GPU {gpu_id}')

    # Adding titles and labels
    plt.title('Watts for Each GPUId')
    plt.xlabel('Index')
    plt.ylabel('Watts')
    plt.legend(title='GPUId')
    plt.grid(True)

    if output_path:
        # Save the plot as PNG
        plt.savefig(output_path)

    plt.show()

if __name__ == "__main__":
    # Check if the file path and output path are provided as arguments
    if len(sys.argv) < 2:
        print("Usage: python script_name.py path_to_csv_file output_path")
        sys.exit(1)

    file_path = sys.argv[1]

    output_path = sys.argv[2] if len(sys.argv) >= 3 else ""

    myplot(file_path, output_path)