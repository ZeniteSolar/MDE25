"""
ADC Linearization Analysis
This script analyzes ADC values and creates a linearization function.
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
from scipy.optimize import curve_fit

# Data from the measurements

"""
real,adc
0.0,63.0
4.97,4339.0
9.93,8700.0
14.91,13069.0
19.90,17439.0
24.89,21766.0
29.88,26120.0
34.87,30357.0
36.37,31605.0
"""

data = """
real,adc
-35.27,16647.000000
-30.19,18761.000000
-25.14,20894.000000
-19.96,23039.000000
-14.92,25196.000000
-9.95,27354.000000
-7.38,28425.000000
-4.93,29497.000000
-2.526,30570.000000
2.448,32707.000000
4.92,33836.000000
7.39,34927.000000
9.91,36015.000000
14.96,38192.000000
19.95,40357.000000
25.08,42520.000000
30.19,44649.000000
35.27,46740.000000
"""

# Parse the data
lines = data.strip().split('\n')[1:]  # Skip header
real_voltages = []
adc_values = []

for line in lines:
    if line.strip():
        real, adc = line.split(',')
        real_voltages.append(float(real))
        adc_values.append(float(adc))

real_voltages = np.array(real_voltages)
adc_values = np.array(adc_values)

# Fit linear function: real_voltage = a * adc_value + b
# This gives us the linearization coefficients
slope, intercept, r_value, p_value, std_err = stats.linregress(adc_values, real_voltages)

# Alternative: Use curve_fit for more control
def linear_func(adc, a, b):
    return a * adc + b

# Fit the function
popt, pcov = curve_fit(linear_func, adc_values, real_voltages)
a_fit, b_fit = popt

# Calculate R-squared
y_pred = linear_func(adc_values, a_fit, b_fit)
residuals = real_voltages - y_pred
ss_res = np.sum(residuals**2)
ss_tot = np.sum((real_voltages - np.mean(real_voltages))**2)
r_squared = 1 - (ss_res / ss_tot)

# Create the plot
plt.figure(figsize=(12, 8))

# Plot 1: Original data and fit
plt.subplot(2, 2, 1)
plt.scatter(adc_values, real_voltages, color='blue', label='Measured Data', s=50)
adc_range = np.linspace(0, max(adc_values), 100)
real_fit = linear_func(adc_range, a_fit, b_fit)
plt.plot(adc_range, real_fit, 'r-', label=f'Linear Fit: y = {a_fit:.4f}x + {b_fit:.4f}')
plt.xlabel('ADC Value')
plt.ylabel('Real Voltage (V)')
plt.title('ADC Linearization')
plt.legend()
plt.grid(True, alpha=0.3)

# Plot 2: Residuals
plt.subplot(2, 2, 2)
residuals = real_voltages - linear_func(adc_values, a_fit, b_fit)
plt.scatter(adc_values, residuals, color='green', s=50)
plt.axhline(y=0, color='r', linestyle='--')
plt.xlabel('ADC Value')
plt.ylabel('Residuals (V)')
plt.title('Fit Residuals')
plt.grid(True, alpha=0.3)

# Plot 3: Inverse function (ADC to Voltage)
plt.subplot(2, 2, 3)
plt.scatter(real_voltages, adc_values, color='purple', label='Measured Data', s=50)
voltage_range = np.linspace(0, max(real_voltages), 100)
adc_fit = (voltage_range - b_fit) / a_fit
plt.plot(voltage_range, adc_fit, 'r-', label=f'Inverse: ADC = (V - {b_fit:.4f}) / {a_fit:.4f}')
plt.xlabel('Real Voltage (V)')
plt.ylabel('ADC Value')
plt.title('Voltage to ADC Conversion')
plt.legend()
plt.grid(True, alpha=0.3)

# Plot 4: Error analysis
plt.subplot(2, 2, 4)
voltage_predicted = linear_func(adc_values, a_fit, b_fit)
error_percent = ((real_voltages - voltage_predicted) / real_voltages) * 100
plt.scatter(real_voltages, error_percent, color='orange', s=50)
plt.axhline(y=0, color='r', linestyle='--')
plt.xlabel('Real Voltage (V)')
plt.ylabel('Error (%)')
plt.title('Percentage Error')
plt.grid(True, alpha=0.3)

plt.tight_layout()
plt.show()

# Print results
print("=" * 60)
print("ADC LINEARIZATION ANALYSIS")
print("=" * 60)
print(f"Number of data points: {len(real_voltages)}")
print(f"Voltage range: {min(real_voltages):.2f}V to {max(real_voltages):.2f}V")
print(f"ADC range: {min(adc_values):.6f} to {max(adc_values):.6f}")
print()

print("LINEARIZATION COEFFICIENTS:")
print("-" * 30)
print(f"Slope (a): {a_fit:.6f}")
print(f"Intercept (b): {b_fit:.6f}")
print(f"R-squared: {r_squared:.6f}")
print(f"Standard error: {std_err:.6f}")
print()

print("LINEARIZATION FUNCTIONS:")
print("-" * 30)
print(f"Voltage = {a_fit:.6f} × ADC + {b_fit:.6f}")
print(f"ADC = (Voltage - {b_fit:.6f}) / {a_fit:.6f}")
print()

print("C CODE IMPLEMENTATION:")
print("-" * 30)
print("// Function to convert ADC value to voltage")
print(f"float adc_to_voltage(float adc_value) {{")
print(f"    return {a_fit:.6f}f * adc_value + {b_fit:.6f}f;")
print("}")
print()
print("// Function to convert voltage to ADC value")
print(f"float voltage_to_adc(float voltage) {{")
print(f"    return (voltage - {b_fit:.6f}f) / {a_fit:.6f}f;")
print("}")
print()

print("ERROR ANALYSIS:")
print("-" * 30)
max_error = np.max(np.abs(error_percent))
min_error = np.min(error_percent)
mean_error = np.mean(error_percent)
std_error = np.std(error_percent)
print(f"Maximum error: {max_error:.3f}%")
print(f"Minimum error: {min_error:.3f}%")
print(f"Mean error: {mean_error:.3f}%")
print(f"Standard deviation of error: {std_error:.3f}%")
print()

# Save coefficients to a file for easy access
with open('linearization_coefficients.txt', 'w') as f:
    f.write("ADC Linearization Coefficients\n")
    f.write("=" * 30 + "\n")
    f.write(f"Slope (a): {a_fit:.6f}\n")
    f.write(f"Intercept (b): {b_fit:.6f}\n")
    f.write(f"R-squared: {r_squared:.6f}\n")
    f.write(f"Standard error: {std_err:.6f}\n\n")
    f.write("C Functions:\n")
    f.write(f"float adc_to_voltage(float adc_value) {{\n")
    f.write(f"    return {a_fit:.6f}f * adc_value + {b_fit:.6f}f;\n")
    f.write("}\n\n")
    f.write(f"float voltage_to_adc(float voltage) {{\n")
    f.write(f"    return (voltage - {b_fit:.6f}f) / {a_fit:.6f}f;\n")
    f.write("}\n")

print("Coefficients saved to 'linearization_coefficients.txt'")
