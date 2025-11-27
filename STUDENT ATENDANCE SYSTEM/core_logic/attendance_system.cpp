#include <iostream> // For input/output operations (e.g., cout, cin)
#include <map>      // For std::map (HashMap equivalent)
#include <vector>   // For std::vector (list of dates)
#include <string>   // For std::string (dates)
#include <set>      // For std::set to count unique dates for statistics
#include <algorithm> // For std::find and std::sort
#include <sstream>  // For std::stringstream to build JSON strings
#include <fstream>  // For file operations (ifstream, ofstream)

// Define the filename for persistent storage
const std::string DATA_FILENAME = "attendance_data.json";

// Define a class to encapsulate the attendance system logic
class AttendanceSystem {
private:
    // Using std::map as a HashMap: roll number (int) -> list of dates (vector of strings)
    std::map<int, std::vector<std::string>> attendance;

    /**
     * @brief Helper to escape strings for JSON output (e.g., handling quotes).
     * @param s The string to escape.
     * @return The escaped string.
     */
    std::string escape_json_string(const std::string& s) const {
        std::string escaped_s = "";
        for (char c : s) {
            if (c == '"') {
                escaped_s += "\\\"";
            } else if (c == '\\') {
                escaped_s += "\\\\";
            } else if (c == '\n') {
                escaped_s += "\\n";
            } else if (c == '\r') {
                escaped_s += "\\r";
            } else if (c == '\t') {
                escaped_s += "\\t";
            } else {
                escaped_s += c;
            }
        }
        return escaped_s;
    }

public:
    // Constructor (optional, can be empty for simple initialization)
    AttendanceSystem() {
        // Data will now be loaded from file, so no dummy data here.
    }

    /**
     * @brief Loads attendance data from a JSON file.
     * @return True if data was loaded successfully, false otherwise.
     */
    bool loadData() {
        std::ifstream inFile(DATA_FILENAME);
        if (!inFile.is_open()) {
            // File doesn't exist or cannot be opened, which is fine for first run.
            // std::cerr << "Warning: Could not open " << DATA_FILENAME << " for reading. Starting with empty data." << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string json_str = buffer.str();
        inFile.close();

        if (json_str.empty()) {
            return false; // Empty file
        }

        // Simple JSON parsing (for demonstration purposes, a full JSON library would be better)
        // Expected format: {"101":["2025-07-01","2025-07-02"], "102":["2025-07-01"]}
        try {
            // Remove outer braces
            if (json_str.length() < 2 || json_str.front() != '{' || json_str.back() != '}') {
                std::cerr << "Error: Invalid JSON format in " << DATA_FILENAME << std::endl;
                return false;
            }
            json_str = json_str.substr(1, json_str.length() - 2);

            std::stringstream ss(json_str);
            std::string segment;
            // Iterate through key-value pairs separated by commas, but be careful with nested commas in arrays
            // This simple parser assumes no commas within date strings and no complex nesting.
            // A robust JSON parser (like nlohmann/json) is highly recommended for real applications.
            size_t start = 0;
            size_t end = 0;
            int brace_count = 0;
            int bracket_count = 0;

            for (size_t i = 0; i < json_str.length(); ++i) {
                if (json_str[i] == '{') brace_count++;
                else if (json_str[i] == '}') brace_count--;
                else if (json_str[i] == '[') bracket_count++;
                else if (json_str[i] == ']') bracket_count--;

                if (json_str[i] == ',' && brace_count == 0 && bracket_count == 0) {
                    segment = json_str.substr(start, i - start);
                    start = i + 1;

                    size_t colon_pos = segment.find(':');
                    if (colon_pos == std::string::npos) continue;

                    std::string key_str = segment.substr(0, colon_pos);
                    std::string value_str = segment.substr(colon_pos + 1);

                    // Extract roll number (remove quotes if present)
                    if (key_str.length() >= 2 && key_str.front() == '"' && key_str.back() == '"') {
                        key_str = key_str.substr(1, key_str.length() - 2);
                    }
                    int rollNo = std::stoi(key_str);

                    // Extract dates array (remove brackets and quotes)
                    if (value_str.length() >= 2 && value_str.front() == '[' && value_str.back() == ']') {
                        value_str = value_str.substr(1, value_str.length() - 2);
                    } else {
                        continue; // Not a valid array
                    }

                    std::stringstream date_ss(value_str);
                    std::string date_segment;
                    while (std::getline(date_ss, date_segment, ',')) {
                        // Remove quotes from date and trim whitespace
                        date_segment.erase(0, date_segment.find_first_not_of(" \t\n\r\f\v"));
                        date_segment.erase(date_segment.find_last_not_of(" \t\n\r\f\v") + 1);
                        if (date_segment.length() >= 2 && date_segment.front() == '"' && date_segment.back() == '"') {
                            date_segment = date_segment.substr(1, date_segment.length() - 2);
                        }
                        attendance[rollNo].push_back(date_segment);
                    }
                    std::sort(attendance[rollNo].begin(), attendance[rollNo].end()); // Ensure loaded dates are sorted
                }
            }
            // Process the last segment
            segment = json_str.substr(start);
            size_t colon_pos = segment.find(':');
            if (colon_pos != std::string::npos) {
                std::string key_str = segment.substr(0, colon_pos);
                std::string value_str = segment.substr(colon_pos + 1);

                if (key_str.length() >= 2 && key_str.front() == '"' && key_str.back() == '"') {
                    key_str = key_str.substr(1, key_str.length() - 2);
                }
                int rollNo = std::stoi(key_str);

                if (value_str.length() >= 2 && value_str.front() == '[' && value_str.back() == ']') {
                    value_str = value_str.substr(1, value_str.length() - 2);
                }

                std::stringstream date_ss(value_str);
                std::string date_segment;
                while (std::getline(date_ss, date_segment, ',')) {
                    date_segment.erase(0, date_segment.find_first_not_of(" \t\n\r\f\v"));
                    date_segment.erase(date_segment.find_last_not_of(" \t\n\r\f\v") + 1);
                    if (date_segment.length() >= 2 && date_segment.front() == '"' && date_segment.back() == '"') {
                        date_segment = date_segment.substr(1, date_segment.length() - 2);
                    }
                    attendance[rollNo].push_back(date_segment);
                }
                std::sort(attendance[rollNo].begin(), attendance[rollNo].end());
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON from " << DATA_FILENAME << ": " << e.what() << std::endl;
            attendance.clear(); // Clear potentially corrupted data
            return false;
        }
    }

    /**
     * @brief Saves attendance data to a JSON file.
     * @return True if data was saved successfully, false otherwise.
     */
    bool saveData() const {
        std::ofstream outFile(DATA_FILENAME);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open " << DATA_FILENAME << " for writing." << std::endl;
            return false;
        }

        outFile << "{";
        bool first_student = true;
        for (const auto& pair : attendance) {
            if (!first_student) {
                outFile << ",";
            }
            outFile << "\"" << pair.first << "\":["; // Roll number as string key
            bool first_date = true;
            for (const std::string& date : pair.second) {
                if (!first_date) {
                    outFile << ",";
                }
                outFile << "\"" << escape_json_string(date) << "\""; // Escape and quote date
                first_date = false;
            }
            outFile << "]";
            first_student = false;
        }
        outFile << "}";
        outFile.close();
        return true;
    }

    /**
     * @brief Marks attendance for a given student on a specific date.
     * @param rollNo The roll number of the student.
     * @param date The date to mark attendance (e.g., "YYYY-MM-DD").
     * @return A JSON string indicating success or error.
     */
    std::string markAttendance(int rollNo, const std::string& date) {
        // Check if the student already has attendance marked for this date
        // Use std::find to check if the date already exists in the vector
        if (std::find(attendance[rollNo].begin(), attendance[rollNo].end(), date) == attendance[rollNo].end()) {
            attendance[rollNo].push_back(date); // Add the date
            std::sort(attendance[rollNo].begin(), attendance[rollNo].end()); // Keep dates sorted
            return "{\"status\": \"success\", \"message\": \"Attendance marked for Roll No: " + std::to_string(rollNo) + " on " + date + "\"}";
        } else {
            return "{\"status\": \"error\", \"message\": \"Attendance already marked for Roll No: " + std::to_string(rollNo) + " on " + date + "\"}";
        }
    }

    /**
     * @brief Views all marked attendance dates for a specific student.
     * @param rollNo The roll number of the student.
     * @return A JSON string with attendance data or an error message.
     */
    std::string viewAttendance(int rollNo) const {
        // Check if the roll number exists in the map
        auto it = attendance.find(rollNo);
        if (it != attendance.end()) {
            std::stringstream ss;
            ss << "{\"status\": \"success\", \"roll_no\": " << rollNo << ", \"dates\": [";
            for (size_t i = 0; i < it->second.size(); ++i) {
                ss << "\"" << escape_json_string(it->second[i]) << "\""; // Enclose dates in quotes for JSON string
                if (i < it->second.size() - 1) {
                    ss << ", ";
                }
            }
            ss << "]}";
            return ss.str();
        } else {
            return "{\"status\": \"error\", \"message\": \"Roll No: " + std::to_string(rollNo) + " not found.\"}";
        }
    }

    /**
     * @brief Calculates and returns overall attendance statistics.
     * @return A JSON string containing statistics.
     */
    std::string getOverallStats() const {
        int totalStudents = attendance.size(); // Number of unique roll numbers

        int totalAttendanceEntries = 0;
        std::set<std::string> uniqueDates; // Use a set to count unique dates across all students

        for (const auto& pair : attendance) {
            totalAttendanceEntries += pair.second.size(); // Sum of all attendance marks
            for (const std::string& date : pair.second) {
                uniqueDates.insert(date); // Add date to set to count unique days
            }
        }

        std::stringstream ss;
        ss << "{\"status\": \"success\", \"stats\": {";
        ss << "\"total_students\": " << totalStudents << ", ";
        ss << "\"total_unique_dates\": " << uniqueDates.size() << ", ";
        ss << "\"total_attendance_entries\": " << totalAttendanceEntries;
        ss << "}}";
        return ss.str();
    }
};

// Main function now acts as a command-line interface for the Flask app
int main(int argc, char* argv[]) {
    // This static instance will persist data across calls within the same process
    // For a real-world scenario, you'd use a database or persistent storage.
    static AttendanceSystem system;

    // Load data at the beginning of each execution
    system.loadData();

    // Check for minimum arguments (command name)
    if (argc < 2) {
        std::cout << "{\"status\": \"error\", \"message\": \"Usage: ./attendance_app <command> [args]\"}" << std::endl;
        return 1; // Indicate error
    }

    std::string command = argv[1]; // The first argument is the command (e.g., "mark", "view", "stats")
    std::string result_json; // To store the JSON output from the function calls

    if (command == "mark") {
        // Expects: ./attendance_app mark <roll_no> <date>
        if (argc != 4) {
            result_json = "{\"status\": \"error\", \"message\": \"Usage: ./attendance_app mark <roll_no> <date>\"}";
        } else {
            try {
                int rollNo = std::stoi(argv[2]); // Convert string argument to integer
                std::string date = argv[3];
                result_json = system.markAttendance(rollNo, date);
                // Save data after modification
                system.saveData();
            } catch (const std::exception& e) {
                result_json = "{\"status\": \"error\", \"message\": \"Invalid roll number or date format: " + std::string(e.what()) + "\"}";
            }
        }
    } else if (command == "view") {
        // Expects: ./attendance_app view <roll_no>
        if (argc != 3) {
            result_json = "{\"status\": \"error\", \"message\": \"Usage: ./attendance_app view <roll_no>\"}";
        } else {
            try {
                int rollNo = std::stoi(argv[2]);
                result_json = system.viewAttendance(rollNo);
            } catch (const std::exception& e) {
                result_json = "{\"status\": \"error\", \"message\": \"Invalid roll number format: " + std::string(e.what()) + "\"}";
            }
        }
    } else if (command == "stats") {
        // Expects: ./attendance_app stats
        if (argc != 2) {
            result_json = "{\"status\": \"error\", \"message\": \"Usage: ./attendance_app stats\"}";
        } else {
            result_json = system.getOverallStats();
        }
    } else {
        // Handle unknown commands
        result_json = "{\"status\": \"error\", \"message\": \"Unknown command: " + command + "\"}";
    }

    std::cout << result_json << std::endl; // Output JSON to stdout
    return 0; // Indicate successful execution
}
