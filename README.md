# PyQuantile

PyQuantile is a Python library that provides a fast quantile estimator for streamed data without storing any data points.

## Installation

You can install PyQuantile via pip:
pip install pyquantile

## Usage

Here is an example of how to use PyQuantile:
import pyquantile

### Create a quantile estimator for the 75th percentile
estimator = pyquantile.QuantileEstimator(0.75)

### Add data points to the stream
estimator.add(10)
estimator.add(20)
estimator.add(30)
etc...

.add() can be called in a loop for streaming data, and .quantile() can be called at any time to get the latest estimate.

### Get current estimate
current_estimate = estimator.quantile()

## Streaming Integration Example

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

## Contributing

Contributions are welcome! If you would like to contribute, please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

