import time
import numpy as np
import matplotlib.pyplot as plt
from pyquantile import QuantileEstimator

# simulate a stream of data for 60 seconds and plot accuracy of quantile estimates over time
duration_seconds = 90
quantile = 0.75
stream = []
estimator = QuantileEstimator(quantile)
accuracies = []
estimates = []
true_quantiles = []
timestamps = []

start = time.time()
while time.time() - start < duration_seconds:
    x = np.random.normal(loc=0, scale=1)
    stream.append(x)
    estimator.add(x)
    current_estimate = estimator.quantile()
    current_true = np.quantile(stream, quantile)
    accuracy = abs(current_estimate - current_true)
    estimates.append(current_estimate)
    true_quantiles.append(current_true)
    accuracies.append(accuracy)
    timestamps.append(time.time() - start)
    time.sleep(0.01)

plt.figure(figsize=(10, 6))
plt.plot(timestamps, estimates, label=f'Estimated {quantile} Quantile')
plt.plot(timestamps, true_quantiles, label=f'True {quantile} Quantile')
plt.plot(timestamps, accuracies, label='Absolute Error', color='green')
plt.xlabel('Time (s)')
plt.ylabel('Value')
plt.title('Quantile Estimation Accuracy Over Time')
plt.legend()
plt.show()
