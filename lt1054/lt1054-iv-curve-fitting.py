import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

# Define the fitting functions
def func3(x, a, b, c, d):
    return a * x**3 + b * x**2 + c * x + d

def func2(x, a, b, c):
    return a * x**2 + b * x + c

def func1(x, a, b):
    return a * x + b

# Load the data from the CSV file
data = np.loadtxt("lt1054-data.csv", delimiter=",")

# Extract the x and y data from the data array
x = data[:, 0]
y = data[:, 1]

# Fit the functions to the data using curve_fit
popt3, _ = curve_fit(func3, x, y)
popt2, _ = curve_fit(func2, x, y)
popt1, _ = curve_fit(func1, x, y)

# Plot the data and the fitted functions
plt.plot(x, y, 'o', label='data')
plt.plot(x, func3(x, *popt3), label='3rd-degree polynomial fit')
plt.plot(x, func2(x, *popt2), label='2nd-degree polynomial fit')
plt.plot(x, func1(x, *popt1), label='linear fit')
plt.legend()

plt.xlabel('I, mA')
plt.ylabel('V')
plt.title('Fitted Function')
plt.show()

print("Fitted parameters for 3rd-degree polynomial: ", popt3)
print("Fitted parameters for 2nd-degree polynomial: ", popt2)
print("Fitted parameters for linear function: ", popt1)

# Fitted parameters for 3rd-degree polynomial:  [-1.29032884e-03  1.68607681e-02 -1.21901634e-01  4.81152883e+00]
# Fitted parameters for 2nd-degree polynomial:  [ 8.12178175e-04 -6.48985019e-02  4.76202325e+00]
# Fitted parameters for linear function:  [-0.05870862  4.75386684]