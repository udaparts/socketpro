
import threading

class UFuture(object):

	# States for Future.
	PENDING = 0
	COMPLETED = 1
	CANCELLED = 2

	def __init__(self, ash = None):
		self._lock_ = threading.Lock()
		self._cv_ = threading.Condition(self._lock_)
		self._state_ = UFuture.PENDING
		self._v_ = None
		self._ash_ = ash

	def set(self, v):
		with self._lock_:
			if self._state_ <= UFuture.COMPLETED:
				self._v_ = v
				self._state_ = UFuture.COMPLETED
			self._cv_.notify_all()

	def cancel(self, mayInterruptIfRunning = True):
		canceled = False
		with self._lock_:
			while True:
				if self._state_ >= UFuture.COMPLETED:
					break
				if mayInterruptIfRunning and self._ash_:
					cs = self._ash_.AttachedClientSocket
					if cs:
						cs.Cancel()
				self._state_ = UFuture.CANCELLED
				canceled = True
				self._cv_.notify_all()
				break;
		return canceled

	@property
	def canceled(self):
		with self._lock_:
			return self._state_ >= UFuture.CANCELLED

	@canceled.setter
	def canceled(self, v):
		if not v:
			return
		with self._lock_:
			if self._state_ < UFuture.COMPLETED:
				self._state_ = UFuture.CANCELLED
			self._cv_.notify_all()

	@property
	def state(self):
		with self._lock_:
			return self._state_

	@property
	def done(self):
		with self._lock_:
			return self._state_ >= UFuture.COMPLETED

	def get(self, timeout):
		with self._lock_:
			if self._state_ == UFuture.PENDING:
				self._cv_.wait(timeout)
			return self._v_
