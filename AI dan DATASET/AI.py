import streamlit as st
import firebase_admin
from firebase_admin import credentials, db
import pandas as pd
import numpy as np
import time
from PIL import Image
from sklearn.linear_model import LinearRegression
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
import matplotlib.pyplot as plt


favicon = Image.open("image.ico")  

st.set_page_config(
    page_title="Real Time Data",
    page_icon=favicon,
    layout="wide",
    initial_sidebar_state="auto"
)


inputSuhuAir = st.text_input("Suhu Air")
inputSuhuApi = st.text_input("Suhu Api")
inputTekanan = st.text_input("Tekanan")
inputBukaan = st.slider("0.00 berarti katup tertutup, 1.00 berarti katup terbuka sempurna (FULL)", 0.001, 1.000)

# Coba konversi input ke float
try:
    inputSuhuApi = float(inputSuhuApi)
    inputSuhuAir = float(inputSuhuAir)
    inputTekanan = float(inputTekanan)
except ValueError:
    st.error("Pastikan semua nilai input adalah angka yang valid.")


dataset = pd.read_csv('dataset.csv')

dataset.columns = dataset.columns.str.replace('[', '').str.replace(']', '')

# Konversi ke dictionary
DicDataset = {
    'Suhu air': dataset['suhu air'].values[:61],
    'Suhu api': dataset['suhu api'].values[:61],
    'Tekanan': dataset['tekanan(bar)'].values[:61],
    'Bukaan Katup': dataset['bukaan katup'].values[:61],
    'Tegangan': dataset['tegangan(V)'].values[:61]
}
train_dataset_df = pd.DataFrame(DicDataset)

# Prosesing dataset
X_train = np.array(train_dataset_df[['Suhu air', 'Suhu api', 'Tekanan', 'Bukaan Katup']])
y_train = np.array(train_dataset_df['Tegangan'])

# Training model
model = RandomForestRegressor(n_estimators=1000, random_state=42)
model.fit(X_train, y_train)

model2 = LinearRegression()
model2.fit(X_train, y_train)

# User input
if st.button('Predict'):
    if all(isinstance(i, float) for i in [inputSuhuApi, inputSuhuAir, inputTekanan, inputBukaan]):
        # Konversi input menjadi list float
        X_input_array = [inputSuhuAir, inputSuhuApi, inputTekanan, inputBukaan]

        # Memprediksi nilai menggunakan model
        y_input_predLG = model2.predict([X_input_array])

        st.write("Hasil Predeksi Tegangan(V): ")
        st.write(y_input_predLG)
    else:
        st.error("Pastikan semua nilai input adalah angka yang valid.")
