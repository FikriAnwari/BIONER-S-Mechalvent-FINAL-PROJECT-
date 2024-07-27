import streamlit as st
import firebase_admin
from firebase_admin import credentials, db, auth
import plotly.graph_objects as go
import smtplib
from email.mime.text import MIMEText
import os
import time

st.set_page_config(
    page_title="Dashboard",
    page_icon="⚡",
    layout="wide",
    initial_sidebar_state="collapsed",
)

# Ensure user is logged in
if 'logged_in' not in st.session_state or not st.session_state['logged_in']:
    st.error("Please log in to access this page.")
    st.switch_page('Home.py')

if st.sidebar.button("Logout"):
    st.session_state['logged_in'] = False
    st.switch_page('Home.py')

# Initialize Firebase Admin SDK
@st.cache_resource
def init_firebase():
    if not firebase_admin._apps:
        cred = credentials.Certificate('bioner-s-firebase-adminsdk-mz6mu-bf7339b2ed.json')
        firebase_admin.initialize_app(cred, {
            'databaseURL': "https://bioner-s-default-rtdb.asia-southeast1.firebasedatabase.app/"
        })

# Fetch real-time data from Firebase
def fetch_data():
    suhuApi = db.reference('/SuhuApi').get()
    suhuAir = db.reference('/SuhuAir').get()
    volt = db.reference('/Volt').get()
    ampere = db.reference('/Ampere').get()
    watt = db.reference('/Watt').get()
    blower = db.reference('/Blower').get()
    tekanan_uap = db.reference('/Tekanan_uap').get()
    return suhuApi, suhuAir, volt, ampere, watt, blower, tekanan_uap

# Fetch the logged-in user's email
def get_user_email():
    user = auth.get_user(st.session_state['user_id'])
    return user.email

# Send email notification using smtplib
def send_email(receiver_email, subject, body):
    sender_email = 'aqsha.rhizqa.endhisza@gmail.com'
    sender_password = 'tjvv mdqc onqa ikqe'

    if not sender_email or not sender_password:
        st.error("Email credentials are not set in environment variables.")
        return

    msg = MIMEText(body)
    msg['Subject'] = subject
    msg['From'] = sender_email
    msg['To'] = receiver_email

    try:
        with smtplib.SMTP_SSL('smtp.gmail.com', 465) as server:
            server.login(sender_email, sender_password)
            server.sendmail(sender_email, receiver_email, msg.as_string())
        st.success(f"Tekanan Uap Mencukupi")
    except smtplib.SMTPAuthenticationError as e:
        st.error(f"SMTP Authentication Error: {e}")
    except smtplib.SMTPException as e:
        st.error(f"SMTP Error: {e}")
    except Exception as e:
        st.error(f"Failed to send email: {e}")

# Function to create a radial bar
def create_radial_bar(value, title, max_value=500):
    percentage = (value / max_value) * 100
    if percentage > 100:
        percentage = 100
    elif percentage < 0:
        percentage = 0
    
    fig = go.Figure(go.Pie(
        hole=0.7,
        sort=False,
        direction='clockwise',
        values=[percentage, 100 - percentage],
        marker=dict(colors=["gold", "lightgray"]),
        textinfo='none'
    ))
    fig.update_layout(
        showlegend=False,
        width=200,
        height=200,
        margin=dict(t=10, b=10, l=10, r=10),
        annotations=[dict(text=f'{percentage:.1f}%', x=0.5, y=0.5, font_size=20, showarrow=False)]
    )
    return fig

# Initialize Firebase
init_firebase()

st.title("Bioner-S")
st.subheader("Smart Energy for the Future")

# Fetch data from Firebase
suhuApi, suhuAir, volt, ampere, watt, blower, tekanan_uap = fetch_data()

# Check Tekanan Uap and send email if 9 or above
if tekanan_uap >= 9:
    receiver_email = get_user_email()
    send_email(receiver_email, "Tekanan Uap Mencukupi", "Tekanan uap mencapai atau melebihi 9 Bar.")

# Create radial bars
radial_bars = [
    ("Suhu Air", "°C", suhuAir),
    ("Tegangan Listrik", "V", volt),
    ("Suhu Api", "°C", suhuApi),
]

# Top row with 3 radial bars
col1, col2, col3 = st.columns(3)
with col1:
    st.plotly_chart(create_radial_bar(radial_bars[0][2], radial_bars[0][0]), use_container_width=True)
    st.markdown(f"<div style='text-align: center; font-weight: bold;'>{radial_bars[0][0]}: {radial_bars[0][2]} {radial_bars[0][1]}</div>", unsafe_allow_html=True)

with col2:
    st.plotly_chart(create_radial_bar(radial_bars[1][2], radial_bars[1][0]), use_container_width=True)
    st.markdown(f"<div style='text-align: center; font-weight: bold;'>{radial_bars[1][0]}: {radial_bars[1][2]} {radial_bars[1][1]}</div>", unsafe_allow_html=True)

with col3:
    st.plotly_chart(create_radial_bar(radial_bars[2][2], radial_bars[2][0]), use_container_width=True)
    st.markdown(f"<div style='text-align: center; font-weight: bold;'>{radial_bars[2][0]}: {radial_bars[2][2]} {radial_bars[2][1]}</div>", unsafe_allow_html=True)

# Horizontal rule
st.markdown("---")

# Bottom status section
col1, col2 = st.columns([1, 0.9])

with col1:
    st.markdown(f"""
    **Status**
    - Blower: {"On" if blower else "Off"}
    - Tekanan Uap: {tekanan_uap} Bar
    """)

time.sleep(0.1)
st.rerun()
