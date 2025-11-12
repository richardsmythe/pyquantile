# PyQuantile

Pyquantile's main goal is to estimate a given quantile, with next to no overhead, on streaming data. The idea is that it must satisfy these conditions:

- Provide extremely fast estimation for streaming data, suitable for real-time analytics and large-scale applications.
- Low overhead regardless of the size of the data stream, with minimal CPU usage per update.
- Optimized for maximum throughput and minimize latency.
- Achieve strong accuracy for most quantiles and distributions, with error rates that are acceptable for real-world use cases.
- Incoming datapoints in the stream cannot be stored, making the estimator ideal for environments with strict memory or privacy constraints.

PyQuantile is a modified implementation of the P² algorithm. It dynamically estimates the p-th quantile of a stream of incoming data points without storing them, maintaining just a few markers and adjusting them as the data comes in.

So far PyQuantile demonstrates good performance characteristics for streaming quantile estimation. Memory usage remains constant (O(1)) regardless of data volume, using only about 2 MiB of base memory with no growth even after processing millions of values. Processing speed is impressive at 1.2-2.0 million values per second with consistent sub-millisecond latency (0.001-0.002ms per operation).

- Initial memory allocation only ~2.0 MiB, which is a one-time cost
- Different quantile values (0.25 to 0.99) uses the same memory
- Processing 100 values: No additional memory
- Processing 1,000 values: No additional memory
- Processing 10,000 values: No additional memory
- Processing 100,000 values: No additional memory
- Processing 1,000,000 values: Only 0.13 MiB increase

The graph below shows how PyQuantile gets more efficient with larger data sizes. Peak performance reaches about 2 million values per second at the largest data size. The latency is generally very stable regardless of the data size.
<img width="1190" height="488" alt="image" src="https://github.com/user-attachments/assets/46f97a5f-7e44-41fb-bd25-c8c82a417536" />

PyQuantile is accurate for central quantiles but less reliable at the distribution tails. This is a common for streaming quantile estimators, which often struggle with extreme quantiles.

<img width="991" height="584" alt="image" src="https://github.com/user-attachments/assets/1f1afd3e-478a-4ecb-8047-913f38671f4e" />

PyQuantile is  adaptive, which is visible in the plots below. When the distribution suddenly shifts from N(0,1) to N(2,0.5) at t=10s, PyQuantile's blue line quickly tracks the new quantile value (jumping to ~2.5). To compare it with another estimator (albeit different) T-Digest's red line gradually drifts upward as it accumulates the new data with all historical observations. Similarly, during gradual concept drift (bottom-left plot), PyQuantile stays close to the current distribution's true quantile, maintaining a relatively constant offset, whereas T-Digest represents the aggregate quantile of all data seen so far, causing it to lag behind the actual current quantile. This makes PyQuantile ideal for non-stationary data streams where recent observations are more relevant than historical ones.

<img width="1899" height="1009" alt="image" src="https://github.com/user-attachments/assets/e779b11d-55d3-47dc-8ae5-59a4c49830a5" />



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

