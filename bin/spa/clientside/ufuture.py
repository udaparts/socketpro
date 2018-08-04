
from concurrent.futures import Future

class UFuture(Future):
	def set(self, v):
		self.set_result(v)

	def get(self, timeout=None):
		return self.result(timeout)