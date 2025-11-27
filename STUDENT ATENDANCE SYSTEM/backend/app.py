import json
import subprocess
import os
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS

# Initialize Flask app
app = Flask(__name__, static_folder='../frontend', static_url_path='/')
CORS(app)

# --- Configuration for C++ Executable ---
# Compile C++: g++ ../core_logic/attendance_system.cpp -o ../core_logic/attendance_app
CPP_EXECUTABLE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'core_logic', 'attendance_app'))

# --- Helper function to call the C++ executable ---
def call_cpp_logic(command, *args):
    try:
        cmd_list = [CPP_EXECUTABLE_PATH, command] + list(args)
        result = subprocess.run(cmd_list, capture_output=True, text=True, check=True)
        return json.loads(result.stdout)
    except FileNotFoundError:
        return {"status": "error", "message": f"C++ executable not found at: {CPP_EXECUTABLE_PATH}. Please compile it."}
    except subprocess.CalledProcessError as e:
        try:
            return json.loads(e.stdout)
        except json.JSONDecodeError:
            return {"status": "error", "message": f"C++ error: {e.stderr or e.stdout}"}
    except json.JSONDecodeError:
        return {"status": "error", "message": f"Failed to parse JSON from C++ output. Raw output: {result.stdout}"}
    except Exception as e:
        return {"status": "error", "message": f"Unexpected error: {str(e)}"}

# --- Flask Routes ---

@app.route('/')
def serve_index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/mark_attendance', methods=['POST'])
def mark_attendance_api():
    data = request.get_json()
    if not data or 'roll_no' not in data or 'date' not in data:
        return jsonify({"status": "error", "message": "Missing roll_no or date"}), 400

    roll_no = data['roll_no']
    date = data['date']

    if not isinstance(roll_no, int) or roll_no <= 0:
        return jsonify({"status": "error", "message": "Invalid roll number"}), 400
    if not isinstance(date, str) or len(date) != 10 or date[4] != '-' or date[7] != '-':
        return jsonify({"status": "error", "message": "Invalid date format. Use YYYY-MM-DD"}), 400

    result = call_cpp_logic("mark", str(roll_no), date)
    if result.get("status") == "success":
        return jsonify(result), 200
    if "already marked" in result.get("message", ""):
        return jsonify(result), 409
    return jsonify(result), 500

@app.route('/view_attendance/<int:roll_no>', methods=['GET'])
def view_attendance_api(roll_no):
    if roll_no <= 0:
        return jsonify({"status": "error", "message": "Invalid roll number"}), 400

    result = call_cpp_logic("view", str(roll_no))
    if result.get("status") == "success":
        return jsonify(result), 200
    if "not found" in result.get("message", "").lower():
        return jsonify(result), 404
    return jsonify(result), 500

@app.route('/get_overall_stats', methods=['GET'])
def get_overall_stats_api():
    result = call_cpp_logic("stats")
    if result.get("status") == "success":
        return jsonify(result), 200
    return jsonify(result), 500

# --- Main ---
if __name__ == "__main__":
    print("--- Flask Server for Student Attendance System ---")
    print(f"C++ app path: {CPP_EXECUTABLE_PATH}")
    print("Compile command (Windows): g++ attendance_system.cpp -o attendance_app.exe")
    print("Server running at http://127.0.0.1:5000/")
    app.run(debug=True)
