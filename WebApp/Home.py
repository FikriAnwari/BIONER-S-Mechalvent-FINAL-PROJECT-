import streamlit as st
from streamlit_extras.switch_page_button import switch_page
from streamlit_extras.add_vertical_space import add_vertical_space
import firebase_admin
from firebase_admin import credentials, auth
import requests
import re

# Initialize Firebase
if not firebase_admin._apps:
    cred = credentials.Certificate('bioner-s-firebase-adminsdk-mz6mu-bf7339b2ed.json')
    #firebase_admin.initialize_app(cred)

# Function to hide the sidebar
def hide_sidebar():
    st.markdown("""
        <style>
        [data-testid="stSidebar"] {
            display: none;
        }
        </style>
        """, unsafe_allow_html=True)

st.set_page_config(
    page_title="Bioner-S",
    page_icon="⚡",
    layout="centered",
    initial_sidebar_state="collapsed",
)

with st.container():
    # logo bioner-s
    left_co, cent_co, last_co = st.columns(3)
    with cent_co:
        st.image('NO BG.png', width=300)

# Authentication functions
def register_user(email, password):
    try:
        user = auth.create_user(
            email=email,
            password=password,
        )
        st.success("User registered successfully!")
        return user
    except firebase_admin.auth.EmailAlreadyExistsError:
        st.error("The email address is already in use.")
    except Exception as e:
        st.error(f"Error registering user: {e}")
    return None

def login_user(email, password):
    try:
        api_key = "AIzaSyCgfHmF3FtXcArbBnzcQ_TK45qTcNWRpnA"
        url = f"https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key={api_key}"
        data = {
            "email": email,
            "password": password,
            "returnSecureToken": True
        }
        response = requests.post(url, json=data)
        if response.status_code == 200:
            st.success("Login successful!")
            user_data = response.json()
            user_id = user_data['localId']
            st.session_state['logged_in'] = True
            st.session_state['user_id'] = user_id
            st.session_state['email'] = email
            switch_page("Dashboard")
        else:
            st.error(f"Error: {response.json()['error']['message']}")
    except Exception as e:
        st.error("klik login sekali lagi")

# Email validation function
def is_valid_email(email):
    pattern = r'^[\w\.-]+@[a-zA-Z\d\.-]+\.[a-zA-Z]{2,}$'
    return re.match(pattern, email) is not None

# User Interface
if 'logged_in' not in st.session_state:
    st.session_state['logged_in'] = False

if st.session_state['logged_in']:
    st.sidebar.success("Login successful!")
    switch_page("Dashboard")
else:
    hide_sidebar()
    selection = st.selectbox("Choose an action", ["Login", "Register"], label_visibility="hidden")

    add_vertical_space(2)

    if selection == "Login":
        email = st.text_input('Email Address', key='login_email', placeholder="Enter your email", label_visibility="collapsed")
        password = st.text_input('Password', type='password', key='login_password', placeholder="Enter your password", label_visibility="collapsed")
        if st.button('Login'):
            if email and password:
                login_user(email, password)
            else:
                st.error("Please fill in both fields.")
        
    elif selection == "Register":
        username = st.text_input('Username', key='register_username', placeholder="Enter a username", label_visibility="collapsed")
        email = st.text_input('Email Address', key='register_email', placeholder="Enter your email", label_visibility="collapsed")
        password = st.text_input('Password', type='password', key='register_password', placeholder="Enter a password", label_visibility="collapsed")
        if st.button('Register'):
            if not username or not email or not password:
                st.error("All fields are required.")
            elif not is_valid_email(email):
                st.error("Please enter a valid email address.")
            elif register_user(email, password):
                st.markdown('Please login using your email and password')
                st.balloons()
