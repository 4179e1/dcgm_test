# Compile

Compile the C program by

```
make
```

# Fetch the instant power usage

```
time ./dcgm_test [loopIntervalUsec] [loopDurationUsec] [loops] > result.csv
#or with default values
time ./dcgm_test > result.csv
```

Default values:
- loopIntervalUsec = 1000, or 1 millesecond : Frequency of DCGM sampling
- loopDurationUsec = 1000000, or 1 second : The frequency we are going to pull the data
- loops = 2 : number of loopDurationUsec

Note: make sure you have DCGM running `systemctl status nvidia-dcgm`

# Plot the data

Run the `plot.ipynb`


# Issues

Looks like the highest resolution of `DCGM_FI_DEV_POWER_USAGE_INSTANT` is 50 milliseconds, even if `loopIntervalUsec` were set to 1 millesecond.
For example, approximately 42 samples can be captured in 2 seconds.

![](./result.png)
