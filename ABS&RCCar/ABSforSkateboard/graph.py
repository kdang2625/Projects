import numpy as np
import pandas as pd
import plotly.express as px
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import plotly.graph_objects as go

# Generate sample data
samples_per_category = 34  # Adjust to balance categories
data_low = pd.DataFrame({
    'PWM Duty': np.random.uniform(10, 30, samples_per_category),
    'PWM Amp': np.random.uniform(1, 5, samples_per_category),
    'PWM Freq': np.random.uniform(100, 1000, samples_per_category),
    'Braking Distance': np.random.uniform(5, 50, samples_per_category),
    'Lateral Skew': np.random.uniform(-2, 2, samples_per_category)
})

data_medium = pd.DataFrame({
    'PWM Duty': np.random.uniform(30, 60, samples_per_category),
    'PWM Amp': np.random.uniform(1, 5, samples_per_category),
    'PWM Freq': np.random.uniform(100, 1000, samples_per_category),
    'Braking Distance': np.random.uniform(5, 50, samples_per_category),
    'Lateral Skew': np.random.uniform(-2, 2, samples_per_category)
})

data_high = pd.DataFrame({
    'PWM Duty': np.random.uniform(60, 90, samples_per_category),
    'PWM Amp': np.random.uniform(1, 5, samples_per_category),
    'PWM Freq': np.random.uniform(100, 1000, samples_per_category),
    'Braking Distance': np.random.uniform(5, 50, samples_per_category),
    'Lateral Skew': np.random.uniform(-2, 2, samples_per_category)
})

# Combine balanced datasets
data = pd.concat([data_low, data_medium, data_high], ignore_index=True)

# Categorize PWM Duty into Low, Medium, High
def categorize_duty(duty):
    if duty < 30:
        return 'Low'
    elif duty < 60:
        return 'Medium'
    else:
        return 'High'

data['PWM Duty Category'] = data['PWM Duty'].apply(categorize_duty)

# Define input (X) and outputs (y1, y2)
X = data[['PWM Duty', 'PWM Amp', 'PWM Freq']]
y1 = data['Braking Distance']
y2 = data['Lateral Skew']

# Standardize features
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# Train-test split
X_train, X_test, y1_train, y1_test = train_test_split(X_scaled, y1, test_size=0.2, random_state=42)
X_train, X_test, y2_train, y2_test = train_test_split(X_scaled, y2, test_size=0.2, random_state=42)

# Train models
model1 = LinearRegression()
model1.fit(X_train, y1_train)

model2 = LinearRegression()
model2.fit(X_train, y2_train)

# Predictions
y1_pred = model1.predict(X_test)
y2_pred = model2.predict(X_test)

# Convert back to original scale for plotting
X_test_original = scaler.inverse_transform(X_test)

# 3D Interactive Scatter Plot for Braking Distance (Using Plotly)
fig1 = px.scatter_3d(data, x='PWM Duty', y='Braking Distance', z='PWM Amp', color='PWM Duty Category',
                     title='Braking Distance vs PWM Parameters (Duty Categories)',
                     labels={'PWM Duty': 'PWM Duty (%)', 'Braking Distance': 'Braking Distance (m)', 'PWM Amp': 'PWM Amp (A)'},
                     color_discrete_map={'Low': 'blue', 'Medium': 'green', 'High': 'red'})

# Add Regression line for Braking Distance
X_range = np.linspace(X_test_original[:, 0].min(), X_test_original[:, 0].max(), 100)
X_dummy = np.column_stack((
    X_range,
    np.full(X_range.shape, np.mean(X_test_original[:, 1])),
    np.full(X_range.shape, np.mean(X_test_original[:, 2]))
))

X_dummy_scaled = scaler.transform(X_dummy)
y1_line = model1.predict(X_dummy_scaled)

fig1.add_traces(go.Scatter3d(
    x=X_range, y=y1_line, z=np.full_like(X_range, np.mean(X_test_original[:, 1])),
    mode='lines', name='Regression Line', line=dict(color='black', width=3)
))

# Show the interactive figure
fig1.show()

# 3D Interactive Scatter Plot for Lateral Skew (Using Plotly)
fig2 = px.scatter_3d(data, x='PWM Duty', y='Lateral Skew', z='PWM Amp', color='PWM Duty Category',
                     title='Lateral Skew vs PWM Parameters (Duty Categories)',
                     labels={'PWM Duty': 'PWM Duty (%)', 'Lateral Skew': 'Lateral Skew (m)', 'PWM Amp': 'PWM Amp (A)'},
                     color_discrete_map={'Low': 'blue', 'Medium': 'green', 'High': 'red'})

# Add Regression line for Lateral Skew
y2_line = model2.predict(X_dummy_scaled)

fig2.add_traces(go.Scatter3d(
    x=X_range, y=y2_line, z=np.full_like(X_range, np.mean(X_test_original[:, 1])),
    mode='lines', name='Regression Line', line=dict(color='black', width=3)
))

# Show the interactive figure
fig2.show()

# Display regression equations
coeffs1 = model1.coef_
intercept1 = model1.intercept_
coeffs2 = model2.coef_
intercept2 = model2.intercept_

# Equation for Braking Distance regression line
print(f"Braking Distance Regression Equation: y = {intercept1:.2f} + "
      f"{coeffs1[0]:.2f}*PWM Duty + {coeffs1[1]:.2f}*PWM Amp + {coeffs1[2]:.2f}*PWM Freq")

# Equation for Lateral Skew regression line
print(f"Lateral Skew Regression Equation: y = {intercept2:.2f} + "
      f"{coeffs2[0]:.2f}*PWM Duty + {coeffs2[1]:.2f}*PWM Amp + {coeffs2[2]:.2f}*PWM Freq")

# Generate random data for 34 tests
data = pd.DataFrame({
    'PWM Duty': np.random.uniform(10, 90, samples_per_category),  # Random PWM duty in the full range
    'PWM Amp': np.random.uniform(1, 5, samples_per_category),
    'PWM Freq': np.random.uniform(100, 1000, samples_per_category),
    'Braking Distance': np.random.uniform(5, 50, samples_per_category),
    'Lateral Skew': np.random.uniform(-2, 2, samples_per_category)
})

# Adjust the index to start from 1
data.index = np.arange(1, len(data) + 1)

# Display the data
print(data)

# Optionally save the data to a CSV file to view in a spreadsheet
data.to_csv("generated_test_data.csv", index=True)