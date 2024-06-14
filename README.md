# Compile

Compile the C program by

```
make
```

# Fetch the instant power usage

```
time ./dcgm_test > result.csv
```

Note: make sure you have DCGM running `systemctl status nvidia-dcgm`

# Plot the data

```
python myplot.py result.csv result.png
```

It generates the figure `result.png`

Or you can directly execut and view it from the plot.ipynb


# Issues

Looks like the resolution of `DCGM_FI_DEV_POWER_USAGE_INSTANT` is 100 milliseconds.
Approximately 21 samples can be captured in 2 seconds.

![](./result.png)
