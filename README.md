# PyQuantile

PyQuantile is a Python library that provides a fast quantile estimator for streamed data. It dynamically estimates the p-th quantile of a stream of incoming data points without storing them. Based on p-Squared algorithm for adjusting and updating markers. This is a work in progress and has some issues that need addressing.

- Small initial memory allocation (~2 MiB)
- Handles millions of values with minimal memory impact
- Memory usage is O(1) - constant space complexity
- Perfect for long-running applications or large data streams
- No memory leaking, memory does not grow over time
- No memory accumulation with more data
  
## Performance
- Initial memory allocation only ~2.0 MiB, which is a one-time cost
- Different quantile values (0.25 to 0.99) uses the same memory
- Processing 100 values: No additional memory
- Processing 1,000 values: No additional memory
- Processing 10,000 values: No additional memory
- Processing 100,000 values: No additional memory
- Processing 1,000,000 values: Only 0.13 MiB increase

The graph below shows how PyQuantile actually gets more efficient with larger data sizes. Peak performance reaches about 2 million values per second at the largest data size. The latency is generally very stable regardless of the data size.
<img width="1190" height="488" alt="image" src="https://github.com/user-attachments/assets/46f97a5f-7e44-41fb-bd25-c8c82a417536" />

Accuracy will always fluctuate for streaming algorithms. These are estimates based on changing data with no known size, and accuracy depends heavily on the characteristics of the data. In my tests, the best results tend to be with uniform and beta distributions. Individual quantiles also show varying accuracy:

- 0.25 quantile: Good (7–23% error)
- 0.50 quantile: Varies dramatically by distribution
- 0.75 quantile: Good (8–23% error)
- Upper quantiles (0.90, 0.95, 0.99): Struggles somewhat, especially with skewed or multi-modal data

This means PyQuantile is most reliable for quartiles in well-behaved distributions, but accuracy can drop for extreme quantiles or complex data shapes.

Below is a plot that shows that at the start, the error is high because the estimator has seen very little data.
As more data arrives, both the estimated and true quantiles stabilize, and the error decreases.
Over time, the estimator tracks the true quantile reasonably well, but there is always some error due to the streaming/approximate nature of the algorithm.
<img width="871" height="559" alt="image" src="https://github.com/user-attachments/assets/237a232d-ada4-4b52-8f45-69d539c94ea5" />

## Installation

You can install PyQuantile via pip:

`pip install pyquantile`

## Usage
Using PyQuantile is simple. Here is an example of how to import it and add samples to an estimator:

```python
import pyquantile as pq
estimator = pq.QuantileEstimator(0.75)
```
### Samples are added to an estimator like this:
```python
estimator.add(10)
estimator.add(20)
estimator.add(30)
```
We can add samples via a loop, and `.quantile()` can be called at any time to get the latest estimate. The following code will simulate a stream of data for 60 seconds and plot accuracy of quantile estimates over time:
```python
duration_seconds = 60
quantile = 0.75
stream = []
estimator = pq.QuantileEstimator(quantile)
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
```

## Kafka Integration Example

You can use PyQuantile in real-time streaming scenarios. For example, to process data from a Kafka topic:

```python
from pyquantile import QuantileEstimator
from kafka import KafkaConsumer
import json

estimator = QuantileEstimator(0.75)
consumer = KafkaConsumer('my_topic', bootstrap_servers='localhost:9092')

for message in consumer:
	data_point = float(json.loads(message.value))
	estimator.add(data_point)
	print("Current estimate:", estimator.quantile())
```

You can adapt this pattern for other streaming services (RabbitMQ, AWS Kinesis, etc.) by feeding each incoming data point to `estimator.add()`.

## Development

To build and install the library locally, clone the repository and run:
git clone https://github.com/richardsmythe/pyquantile.git
cd pyquantile
pip install .

## Demo
A python file named `test_pyquantile.py` is included. This file demonstrates PyQuantile in action and plots the results against the exact quantile. Run it with `python .\test_pyquantile.py`. This simple script will show how PyQuantile performs against an exact quantile. With more data the absolute errors begin to even out the estimator becomes more accurate.

## Contributing

Contributions are welcome! If you would like to contribute, please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

