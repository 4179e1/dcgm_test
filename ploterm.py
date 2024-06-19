import pandas as pd
import plotext as plt
import sys


def myplot(filename, gpu_filters=None):
    # Load the CSV file
    data = pd.read_csv(filename)

    # Get unique GPU IDs
    if gpu_filters:
        gpu_ids = gpu_filters
    else:
        gpu_ids = data['GPUId'].unique()

    # Plot the data for each GPU
    plt.clear_figure()
    #plt.figure(figsize=(120, 30))

    for gpu_id in gpu_ids:
        gpu_data = data[data['GPUId'] == gpu_id].reset_index(drop=True)  # Reindex each gpu_data starting from 0
        timestamps = gpu_data['TimeStampUs']
        plt.plot(timestamps, gpu_data['Watts'], label=f'GPU {gpu_id}', marker='dot')

    # Add labels and title
    plt.xlabel('Timestamp')
    plt.ylabel('Watts')
    plt.title('Power Consumption Over Time for Each GPU')

    # Show the plot
    plt.show()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("""Usage: python ploterm.py <filename> [GPU IDs]
Example: ploterm.py result.csv 0,1,2 """)
        exit(1)
    else:
        filename = sys.argv[1]
        _filters = None
        if len(sys.argv) > 2:
            _filters = sys.argv[2].split(',')
            _filters = [int(x) for x in _filters]
        
        myplot(filename, _filters)
