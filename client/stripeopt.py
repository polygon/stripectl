import numpy as np
import matplotlib.pyplot as plt

seglen = np.arange(240)
update_time = seglen * 1.25 + 50.
fps = 1. / (update_time / 1000000.)

plt.plot(seglen, fps)
plt.show()