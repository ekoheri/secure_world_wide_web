import matplotlib.pyplot as plt

# Data for optimized server
optimized_requests = [500, 1000, 5000, 10000, 50000, 100000]
optimized_rps = [11074.44, 10778.42, 10815.02, 10338.02, 11014.69, 11406.90]
optimized_time_per_request = [0.903, 0.928, 0.925, 0.967, 0.908, 0.877]
optimized_transfer_rate = [35407.93, 34461.46, 34578.49, 33053.40, 35216.90, 36470.89]

# Data for non-optimized server
non_optimized_requests = [500, 1000, 5000, 10000, 50000, 100000]
non_optimized_rps = [9676.42, 10314.81, 8811.24, 11649.92, 10834.21, 12890.64]
non_optimized_time_per_request = [1.033, 0.969, 1.135, 0.858, 0.923, 0.776]
non_optimized_transfer_rate = [30938.09, 35310.94, 31768.49, 34434.85, 32511.76, 38745.32]

# Plot Requests per Second (RPS)
plt.figure(figsize=(10, 5))
plt.plot(optimized_requests, optimized_rps, marker='o', label='Optimized RPS', linestyle='-')
plt.plot(non_optimized_requests, non_optimized_rps, marker='o', label='Non-Optimized RPS', linestyle='--')
plt.title("Requests per Second (RPS) Comparison")
plt.xlabel("Number of Requests")
plt.ylabel("Requests per Second")
plt.legend()
plt.grid(True)
plt.show()

# Plot Time per Request
plt.figure(figsize=(10, 5))
plt.plot(optimized_requests, optimized_time_per_request, marker='o', label='Optimized Time per Request', linestyle='-')
plt.plot(non_optimized_requests, non_optimized_time_per_request, marker='o', label='Non-Optimized Time per Request', linestyle='--')
plt.title("Time per Request Comparison")
plt.xlabel("Number of Requests")
plt.ylabel("Time per Request (ms)")
plt.legend()
plt.grid(True)
plt.show()

# Plot Transfer Rate
plt.figure(figsize=(10, 5))
plt.plot(optimized_requests, optimized_transfer_rate, marker='o', label='Optimized Transfer Rate', linestyle='-')
plt.plot(non_optimized_requests, non_optimized_transfer_rate, marker='o', label='Non-Optimized Transfer Rate', linestyle='--')
plt.title("Transfer Rate Comparison")
plt.xlabel("Number of Requests")
plt.ylabel("Transfer Rate (Kbytes/sec)")
plt.legend()
plt.grid(True)
plt.show()
