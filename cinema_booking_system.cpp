#include <iostream>
#include <iomanip>
#include <cctype>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>


#define MAX_LENGTH 50
#define SEATS_X 15
#define SEATS_Y 5
#define TOTAL_SEATS SEATS_X * SEATS_Y
#define BITS 8
#define MAX_FOOD_QUAN 20
#define SST 0.06


#define SEATS_FILE "seats.bin"
#define SALES_FILE "sales"


struct FoodItem;
struct TicketDetails;


void cls();
void display_logo();
void displayMainMenu();
void display_returnable_menus(const std::string title, std::vector<std::string> vector, bool do_back);
void display_seats(int movie_id, int date_id, int time_id, std::vector<int>& selected_seats, bool show_final);
void display_selected_movie_details(int movie_id, int date_id, int time_id);
void display_food_details();
void display_cart();
void clear_food_items(std::vector<FoodItem>& foods);


bool isdigit(std::string s);
char get_character_option(const std::string text, std::string validChars);


int get_numeric_option(const std::string text, int min, int max);
int get_seat_id(int movie_id, int date_id, int time_id, int seat_id);
int get_amount_available_seats(int movie_id, int date_id, int time_id);
int get_seat(int movie_id, int date_id, int time_id, int remaining, std::vector<int>& selected_seats);
int buy_ticket();
void transfer_seats_to_list(int movie_id, int date_id, int time_id, std::vector<int> seats);
void get_payment_method();
void buy_food();
void get_customer_order();
void generate_report(std::vector<double> &daySales, int day, int start_day);
void start_system(int day);
void randomise_seats(std::vector<bool>& seats, int off_weight, int on_weight);


int load_seats();
int save_seats();


int load_sales();
int save_sales();


double calculate_food_price(std::vector<FoodItem> vec);
void display_food_details_receipt(std::vector<FoodItem> vec);
std::string stringify_seats(std::vector<int> vec);
std::string encode_bool(std::vector<bool> vec);
std::string encode_sales(std::vector<double> vec);
std::vector<double> decode_sales(std::string str_data, char sep);
std::vector<bool> decode_bool(std::string str, int max);


struct FoodItem {
        int id = 0;
        int quantity = 0;
};


struct TicketDetails {
        int selected_movie_idx = 0,
                selected_date_idx = 0,
                selected_time_idx = 0,
                adult_pax = 0,
                child_pax = 0,
                member_type = 1;
        std::vector<int> seats = {};
        std::vector<FoodItem> foods = {};
        std::string payment_method = "";


};


/*
https://patorjk.com/software/taag/#p=display&v=3&f=Varsity&t=EBC%20%0A
01234567890123456789012345678901234567890123456789
                  ________   ______       ______
                 |_   __  | |_   _ \    .' ___  |
                   | |_ \_|   | |_) |  / .'   \_|
                   |  _| _    |  __'.  | |
                  _| |__/ |  _| |__) | \ `.___.'\
                 |________| |_______/   `.____ .'
*/


//the global variables
const std::vector<std::string> ALL_MOVIES = {
        "Jurassic Park         |2h 30m",
        "Mat Kilau             |1h 52m",
        "Minecraft             |1h 47m",
        "Aquaman               |2h 47m"
};
const std::vector<std::string> ALL_DATES = { "17 Nov 2025","18 Nov 2025", "20 Nov 2025", "24 Nov 2025", "25 Nov 2025" };
const std::vector<std::string> ALL_TIMES = { "10 AM", "12 PM", "3 PM", "5 PM", "7 PM" };
const std::vector<std::string> PAYMENT_METHODS = { "Credit Card", "e-Bank", "e-Wallet"};
const std::vector<std::string> MEMBER_LIST = { "Premium", "Member", "Normal Customer" };
const std::map<int, std::pair<std::string, double>> FOODS = {
        {0, {"Popcorn (S)", 7}},
        {1, {"Popcorn (L)", 8}},
        {2, {"Chicken hotdog", 6.5}},
        {3, {"Chips", 8.4}},
        {4, {"Sprite", 6}},
        {5, {"Coke", 6}}
};
const std::map<int, std::pair<std::string, double>> MEMBER_TYPES = {
        {0, {"Premium", 0.1}},
        {1, {"Member", 0.05}},
        {2, {"Normal", 0}}
};


std::vector<bool> ALL_SEATS(ALL_MOVIES.size() * ALL_DATES.size() * ALL_TIMES.size() * TOTAL_SEATS);
std::vector<double> ALL_SALES;
std::string DAYS[] = { "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" };
TicketDetails ticket;
double today_sales = 0;


//converts every 8 seats into an unsigned char (eg: {0, 1, 1, 0, 1, 1, 0, 1} -> m)
std::string encode_bool(std::vector<bool> vec) {
        std::string f_str = R"()";
        unsigned char f_c;
        int total_chars = (vec.size() / BITS) + 1;
        int pow_2 = 1;
        for (int c = 0; c < total_chars; c++) {
                pow_2 = 1;
                f_c = 0;
                for (int b = c * BITS; b < (c + 1) * BITS; b++) {
                        if (b < vec.size()) {
                                pow_2 = std::pow(2, b % BITS);
                                f_c += pow_2 * vec.at(b);
                        }
                }
                f_str += f_c;
        }
        return f_str;
}


//decodes the unsigned chars into seats
std::vector<bool> decode_bool(std::string str, int max) {
        std::vector<bool> vec;
        int total_chars = str.length();
        unsigned char c_char;


        for (int c = 0; c < total_chars; c++) {
                c_char = str.at(c);
                for (int b = c * BITS; b < (c + 1) * BITS; b++) {
                        if (b < max) {
                                vec.push_back(c_char % 2);
                        }
                        c_char /= 2;
                }
        }
        return vec;
}


//encode daily sales separated by '/'
std::string encode_sales(std::vector<double> vec) {
        std::string f_str = "";
        for (double sale : vec) {
                f_str += (std::to_string(sale) + "/");
        }
        return f_str;
}


//decode the sales into a vector
std::vector<double> decode_sales(std::string str_data, char sep = '/') {
        std::vector<double> parsed_data;
        std::string temp_str_num = "";


        for (int char_idx = 0; char_idx < str_data.length(); char_idx++) {
                if (str_data.at(char_idx) != sep) {
                        temp_str_num += str_data.at(char_idx);
                }
                else {
                        if (!temp_str_num.empty()) {
                                double num = std::stod(temp_str_num);
                                parsed_data.push_back(num);
                                temp_str_num = "";
                        }
                }
        }
        if (!temp_str_num.empty()) {
                double num = std::stod(temp_str_num);
                parsed_data.push_back(num);
        }


        return parsed_data;
}


//ansi code to scroll up the screen
void cls() {
        std::cout << "\033[2J\033[1;1H" << std::endl;
        // #ifdef _WIN32
        //         system("cls");
        // #else
        //         system("clear");
        // #endif
}


void prompt_enter(){
        std::cout << "Press <Enter> to continue: ";
        std::cin.get();
        std::cout << std::endl;
}


void display_logo() {
        std::cout << std::setfill(' ') << std::setw((MAX_LENGTH - 32) / 2) << " " << " ________   ______       ______ \n";
        std::cout << std::setfill(' ') << std::setw((MAX_LENGTH - 32) / 2) << " " << "|_   __  | |_   _ \\    .' ___  |\n";
        std::cout << std::setfill(' ') << std::setw((MAX_LENGTH - 32) / 2) << " " << "  | |_ \\_|   | |_) |  / .'   \\_|\n";
        std::cout << std::setfill(' ') << std::setw((MAX_LENGTH - 32) / 2) << " " << "  |  _| _    |  __'.  | |       \n";
        std::cout << std::setfill(' ') << std::setw((MAX_LENGTH - 32) / 2) << " " << " _| |__/ |  _| |__) | \\ `.___.'\\\n";
        std::cout << std::setfill(' ') << std::setw((MAX_LENGTH - 32) / 2) << " " << "|________| |_______/   `.____ .'\n";
        std::cout << std::endl;
        std::cout << std::setfill(' ') << std::setw((MAX_LENGTH - 44) / 2) << " " << "Even Better Cinemas, brought to you by H2O.CO\n";
        std::cout << "\n" << std::setfill('-') << std::setw(MAX_LENGTH) << "-" << "\n" << std::endl;
}


void displayMainMenu() {
        std::cout << std::setfill('-') << std::setw((MAX_LENGTH - 10) / 2) << "-" << " MAIN MENU " << std::setw((MAX_LENGTH - 10) / 2) << "-\n";
        std::cout << "1. View Movie Details\n";
        std::cout << "2. Place New Order\n";
        std::cout << "3. Generate Weekly Report\n";
        std::cout << "4. Exit System (End Day)\n";
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-" << std::endl;
}


//displays a vector into a list, also allowing for an optional "Back" option
void display_returnable_menus(const std::string title, std::vector<std::string> vector, bool do_back = true) {
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-\n";
        std::cout << title << "\n";
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-\n";
        for (int i = 0; i < vector.size(); i++) {
                std::cout << i + 1 << ". " << vector.at(i) << "\n";
        }
        if (do_back) {
                std::cout << "0. Back\n";
        }
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-\n" << std::endl;
}


//displays the seat layout of the current room based on its parameters
void display_seats(int movie_id, int date_id, int time_id, std::vector<int>& selected_seats, bool show_final = false) {
        int seat_id = 0;
        std::cout << std::setfill('-') << std::setw((MAX_LENGTH - 12) / 2) << "-" << " Seat Layout " << std::setw((MAX_LENGTH - 12) / 2) << " " << std::setfill(' ') << "\n";
        std::cout << std::setw((MAX_LENGTH - 12) / 2) << " " << "<Screen Here>\n";
        for (int y = 0; y < SEATS_Y; y++) {
                std::cout << std::setw((MAX_LENGTH - (SEATS_X * 2 - 1)) / 2 - 1) << y + 1 << ". ";
                for (int x = 0; x < SEATS_X; x++) {
                        auto it = std::find(selected_seats.begin(), selected_seats.end(), SEATS_X * y + x + 1);
                        if (ALL_SEATS.at(get_seat_id(movie_id, date_id, time_id, SEATS_X * y + x + 1))) {
                                std::cout << "X ";
                        }
                        else if (!selected_seats.empty()){
                                if (it != selected_seats.end() && it != selected_seats.end() - 1) {
                                        std::cout << "S ";
                                }
                                else if (it == selected_seats.end() - 1) {
                                        show_final ? std::cout << "S " : std::cout << "? ";
                                }
                                else {
                                        std::cout << "O ";
                                }
                        }
                        else {
                                std::cout << "O ";
                        }
                }
                std::cout << "\n";
        }
        std::cout << std::setw((MAX_LENGTH - (SEATS_X * 2 - 1)) / 2 + 1) << " ";
        for (int x = 0; x < SEATS_X; x++) {
                //std::cout << seat_repr.at(x) << " ";
                std::cout << static_cast<char>(x + 'A') << " ";
        }
        std::cout << "\n" << std::setfill('-') << std::setw(MAX_LENGTH) << "-" << std::endl;
}


//called when displaying the seat selection screen
void display_selected_movie_details(int movie_id, int date_id, int time_id) {
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-\n";
        std::cout << ALL_MOVIES.at(movie_id - 1) << "|: " << ALL_DATES.at(date_id - 1) << ", " << ALL_TIMES.at(time_id - 1) << std::endl;
}


//display food details in the food buying screen
void display_food_details() {
        std::cout << std::setfill('-') << std::setw((MAX_LENGTH - 10) / 2) << "-" << " FOOD LIST " << std::setw((MAX_LENGTH - 10) / 2) << " " << std::setfill(' ') << "\n";
        for (int f = 0; f < FOODS.size(); f++) {
                std::cout << std::setw(2) << f + 1 << ". " << std::setw(20) << FOODS.at(f).first << " | RM" << std::left << std::setprecision(2) << std::fixed << std::showpoint << FOODS.at(f).second << "\n" << std::right;
        }
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-" << std::setfill(' ') << "\n" << std::endl;
}


//display bought food items in the food buying screen
void display_cart() {
        std::cout << std::setfill('-') << std::setw((MAX_LENGTH - 8) / 2) << "-" << " My Cart " << std::setw((MAX_LENGTH - 8) / 2) << " " << std::setfill(' ') << "\n";
        for (FoodItem food : ticket.foods) {
                if (food.quantity > 0) {
                        std::cout << std::setw(25) << std::left << FOODS.at(food.id).first << "| " << std::right << food.quantity << "x\n";
                }
        }
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-" << std::setfill(' ') << "\n" << std::endl;
}


void display_receipt() {


        int cost = 0;
        std::cout << std::setfill('-') << std::setw((MAX_LENGTH - 8) / 2) << "-" << " Receipt " << std::setw((MAX_LENGTH - 10) / 2) << " " << std::setfill(' ') << "\n";
        
        std::cout << std::left;
        std::cout << "|Member type     : " << std::setw(MAX_LENGTH - 21) << MEMBER_TYPES.at(ticket.member_type).first << "|\n";
        std::cout << "|Movie Name      : " << std::setw(MAX_LENGTH - 21) << ALL_MOVIES.at(ticket.selected_movie_idx - 1) << "|\n";
        std::cout << "|Movie Date      : " << std::setw(MAX_LENGTH - 21) << ALL_DATES.at(ticket.selected_date_idx - 1) << "|\n";
        std::cout << "|Movie Time      : " << std::setw(MAX_LENGTH - 21) << ALL_TIMES.at(ticket.selected_time_idx - 1) << "|\n";
        std::cout << "|" << std::setfill('-') << std::right << std::setw(MAX_LENGTH - 1) << "|\n" << std::setfill(' ') << std::left;


        std::cout << "|Adults   (Pax)  : " << std::setw(MAX_LENGTH - 21) << (std::to_string(ticket.adult_pax) + " x RM 12.00") << "|\n";
        std::cout << "|Children (Pax)  : " << std::setw(MAX_LENGTH - 21) << (std::to_string(ticket.child_pax) + " x RM 6.50") << "|\n";
        std::cout << "|Total Pax       : " << std::setw(MAX_LENGTH - 21) << ticket.adult_pax + ticket.child_pax << "|\n";
        std::cout << "|Seat numbers    : " << std::setw(MAX_LENGTH - 21) << stringify_seats(ticket.seats) << "|\n";
        
        std::cout << "|" << std::setfill('-') << std::right << std::setw(MAX_LENGTH - 1) << "|\n" << std::setfill(' ') << std::left;


        display_food_details_receipt(ticket.foods);
        double food_price = calculate_food_price(ticket.foods);
        double total = food_price + ticket.adult_pax * 12 + ticket.child_pax * 6.5;
        double total_discount = 0.05 + MEMBER_TYPES.at(ticket.member_type).second;
        std::cout << "|Food cost       : RM " << std::left << std::setw(MAX_LENGTH - 24) << std::fixed << std::setprecision(2) << std::showpoint << food_price << "|\n";        
        
        std::cout << "|" << std::setfill('-') << std::right << std::setw(MAX_LENGTH - 1) << "|\n" << std::setfill(' ') << std::left;


        std::cout << "|Subtotal        : RM " << std::setw(MAX_LENGTH - 24) << std::fixed << std::setprecision(2) << std::showpoint << total << "|\n";
        std::cout << "|Tax (6%)        : RM " << std::setw(MAX_LENGTH - 24) << std::fixed << std::setprecision(2) << std::showpoint << total * SST << "|\n";
        total = total * (1 + SST);
        std::cout << "|Total price     : RM " << std::setw(MAX_LENGTH - 24) << std::fixed << std::setprecision(2) << std::showpoint << total << "|\n";
        std::cout << "|Student discount: 5%"  << std::setw(MAX_LENGTH - 23) << " " << "|\n";
        std::cout << "|Total discount  : "    << std::setw(MAX_LENGTH - 21) << (std::to_string(static_cast<int>(total_discount * 100)) + "%") << "|\n";
        std::cout << "|Discounted price: RM " << std::setw(MAX_LENGTH - 24) << std::fixed << std::setprecision(2) << std::showpoint << total * (1 - total_discount) << "|\n";
        std::cout << "|Payment method  : "    << std::setw(MAX_LENGTH - 21) << ticket.payment_method << "|\n";
        
        std::cout << "|" << std::setfill('-') << std::right << std::setw(MAX_LENGTH) << "|\n\n" << std::setfill(' ') << std::left;
        
        std::cout << "Thank you for choosing EBC. Please come again." << std::right << std::endl;
        today_sales += total * (1 - total_discount);
}


//displays the food details in the receipt
void display_food_details_receipt(std::vector<FoodItem> vec) {
        int c = 0;
        std::cout << "|Food details    : " << std::left;
        for (int i = 0; i < vec.size(); i++) {
                if (vec.at(i).quantity > 0) {
                        if (c > 0) {
                                std::cout << "\n|" << std::setw(18) << " "; 
                        }
                        c += 1;
                        std::cout << std::setw(14) << FOODS.at(vec.at(i).id).first << " x " << std::setw(MAX_LENGTH - 38) << vec.at(i).quantity << "|";
                }
        }
        if (c == 0) {
                std::cout << std::setw(MAX_LENGTH - 21) << " " << "|";
        }
        std::cout << std::right << std::endl;
};


//stringify the seats to display in the receipt
std::string stringify_seats(std::vector<int> vec) {
        std::string base = "";
        for (int i = 0; i < vec.size(); i++) {
                base += static_cast<char>(((int)(vec.at(i) - 1) / SEATS_X) + '1');
                base += static_cast<char>(((vec.at(i) - 1) % SEATS_X) + 'A');
                if (i < vec.size() - 1) {
                        base += ",";
                }
        }
        return base;
};


double calculate_food_price(std::vector<FoodItem> vec) {
        double price = 0;
        for (FoodItem food : vec) {
                price += food.quantity * FOODS.at(food.id).second;
        }
        return price;
}


//clear food items whenever a new ticket starts
void clear_food_items(std::vector<FoodItem>& foods) {
        foods = {};
        for (int f = 0; f < FOODS.size(); f++) {
                FoodItem food_item = { f, 0 };
                foods.push_back(food_item);
        }
}


//check if all characters of a string are numeric
bool isdigit(std::string s) {
        for (char c : s) {
                if (!std::isdigit(c)) {
                        return false;
                }
        }
        return true;
}


//gets a valid numeric option between min and max
int get_numeric_option(const std::string text, int min, int max) {
        std::string opt = "";
        int opt_int;
        int max_chars = std::log10(max) + 1;
        do {
                opt_int = min - 1;
                std::cout << text << " (" << min << " - " << max << "): ";
                std::cin >> opt;
                std::cin.ignore(1024, '\n');


                if (opt.length() > max_chars) {
                        std::cout << "Invalid option, input is too long.\n\n";
                        continue;
                }
                if (!isdigit(opt)) {
                        std::cout << "Invalid option, please enter only positive numbers.\n\n";
                        continue;
                }
                opt_int = std::stoi(opt);
                if (opt_int < min || opt_int > max) {
                        std::cout << "Invalid option, out of range.\n\n";
                        continue;
                }
        } while ((opt_int < min || opt_int > max));
        return opt_int;
}


//gets a valid character option based on the given validChars (eg: "YyNn")
char get_character_option(const std::string text, std::string validChars) {
        std::string opt;
        char opt_char;
        while (true) {
                std::cout << text;
                std::cin >> opt;
                std::cin.ignore(1024, '\n');
                if (opt.length() != 1) {
                        std::cout << "Invalid option, option is too long\n\n";
                        continue;
                }
                opt_char = opt.at(0);
                if (validChars.find(opt_char) == std::string::npos) {
                        std::cout << "Invalid option.\n\n";
                        continue;
                }
                return opt_char;
        }
}


int get_seat_id(int movie_id, int date_id, int time_id, int seat_id) {
        /*
        eg a movie_id of 1, date_id of 4 and time_id of 3 as well as seat_id of 25 will be
        (1 - 1) * (5 * 5 * 75) + (4 - 1) * (5 * 75) + (3 - 1) * 75) + (25 - 1)
        = 0 + 1125 + 150 + 25 = 1300 (this is the seat)


        in theory,
        4 * 5 * 5 * 75 = 7500
        each movie will have 1875 seats
        each date will have 375 seats
        each time will have 75 seats
        seat = movie_id * 1875 + date_id * 375 + time_id * 75 + seat_id
        */


        return (
                (movie_id - 1) * (ALL_DATES.size() * ALL_TIMES.size() * TOTAL_SEATS)
                + (date_id - 1) * (ALL_TIMES.size() * TOTAL_SEATS)
                + (time_id - 1) * TOTAL_SEATS
                + (seat_id - 1)
                );
}


//check the amount of available seats in a room
int get_amount_available_seats(int movie_id, int date_id, int time_id) {
        int total_avail_seats = 0;
        for (int i = 0; i < TOTAL_SEATS; i++) {
                if (!ALL_SEATS.at(get_seat_id(movie_id, date_id, time_id, i + 1))) {
                        total_avail_seats++;
                }
        }
        return total_avail_seats;
}


//if got time, try adding swapping seats
int get_seat(int movie_id, int date_id, int time_id, int remaining, std::vector<int>& selected_seats) {
        std::string seat = "";
        int seat_int = 0;
        do {
                std::cout << "(" << remaining << ") seat(s) remaining. Select a seat [S].\nOr, type '0' to retract your latest seat [?].\n[row:col] (e.g. 5B, 3H): ";
                std::cin >> seat;
                std::cin.ignore(1024, '\n');


                //break it down since there's so many conditions
                if (seat == "0") {
                        return 0;
                }
                //input is not 2 chars
                if (seat.length() != 2) {
                        std::cout << "Invalid seat option, please try again.\n\n";
                        continue;
                }
                //input is not in the format (int:char)
                if (!(std::isdigit(seat.at(0)) && std::isalpha(seat.at(1)))) {
                        std::cout << "Invalid seat option, please try again.\n\n";
                        continue;
                }
                //input is not in range
                if (!(seat.at(0) > '0' && seat.at(0) <= SEATS_Y + '0' && std::toupper(seat.at(1)) >= 'A' && std::toupper(seat.at(1)) <= SEATS_X + 'A')) {
                        std::cout << "Invalid seat option, please try again.\n\n";
                        continue;
                }
                int y = (seat.at(0) - '0' - 1), x = (std::toupper(seat.at(1)) - 'A');
                if (ALL_SEATS.at(get_seat_id(movie_id, date_id, time_id, y * SEATS_X + x + 1)) || (std::find(selected_seats.begin(), selected_seats.end(), y * SEATS_X + x + 1) != selected_seats.end())) {
                        std::cout << "Seat is taken, please try again.\n\n";
                        continue;
                }
                seat_int = y * SEATS_X + x + 1;
                std::cout << seat_int << "\n" << std::endl;
        } while (seat_int == 0);
        return seat_int;
}
//long function, due to goto labels to implement jump backs
int buy_ticket() {
        int remaining_seats;
        char confirm_seats_opt;


get_movie:
        ticket.selected_movie_idx = 0;
        cls();
        display_returnable_menus("Now showing:", ALL_MOVIES);
        ticket.selected_movie_idx = get_numeric_option("Please select a movie: ", 0, ALL_MOVIES.size());
        if (ticket.selected_movie_idx == 0) {
                return 1;
        }


get_date:
        ticket.selected_date_idx = 0;
        cls();
        display_returnable_menus("Available dates:", ALL_DATES);
        ticket.selected_date_idx = get_numeric_option("Please select a date: ", 0, ALL_DATES.size());
        if (ticket.selected_date_idx == 0) {
                goto get_movie;
        }


get_time:
        ticket.selected_time_idx = 0;
        cls();
        display_returnable_menus("Showing hours:", ALL_TIMES);
        ticket.selected_time_idx = get_numeric_option("Please select a time: ", 0, ALL_TIMES.size());
        if (ticket.selected_time_idx == 0) {
                goto get_date;
        }


get_pax:
        ticket.adult_pax = 0;
        ticket.child_pax = 0;
        ticket.seats = {};
        cls();
        display_selected_movie_details(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx);
        display_seats(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx, ticket.seats);
        remaining_seats = get_amount_available_seats(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx);


        if (remaining_seats > 0) {
                std::cout << "Please enter '0' for both fields if you wish to\nreturn to the previous menu.\n";
                ticket.adult_pax = get_numeric_option("Please enter the adult pax: ", 0, remaining_seats);
                remaining_seats = get_amount_available_seats(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx) - ticket.adult_pax;
                if (remaining_seats > 0) {
                        ticket.child_pax = get_numeric_option("Please enter the child pax: ", 0, remaining_seats);
                }
                if ((ticket.adult_pax + ticket.child_pax) < 1) {
                        std::cout << "You have entered '0' for both fields, and shall\nnow return to the previous menu.\n";
                        prompt_enter();
                        goto get_time;
                }
                else {
                        goto select_seats;
                }
        }
        else {
                std::cout << "Unfortunately, the seats for this time are full.\nReturning to previous menu.\n";
                prompt_enter();
                goto get_time;
        }


select_seats:
        for (int s = 0; s < ticket.adult_pax + ticket.child_pax; s++) {
                cls();
                display_selected_movie_details(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx);
                display_seats(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx, ticket.seats);
                std::cout << "[X] = Taken, [O] = Available\n\n";
                int seat = get_seat(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx, ticket.adult_pax + ticket.child_pax - s, ticket.seats);
                if (seat > 0) {
                        ticket.seats.push_back(seat);
                }
                else {
                        if (s > 0) {
                                ticket.seats.pop_back();
                                s -= 2;
                        }
                        else {
                                s -= 1;
                        }
                }
        }


        cls();
        display_selected_movie_details(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx);
        display_seats(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx, ticket.seats, true);
        confirm_seats_opt = get_character_option("Are you sure if you want to keep these seats? (Y/N): ", "YyNn");
        if (std::tolower(confirm_seats_opt) == 'n') {
                confirm_seats_opt = ' ';
                goto get_pax;
        }
        return 0;
}


//transfers the seats ids to the ALL_SEATS vector when the user is done ordering
void transfer_seats_to_list(int movie_id, int date_id, int time_id, std::vector<int> seats) {
        for (int seat : seats) {
                ALL_SEATS.at(get_seat_id(movie_id, date_id, time_id, seat)) = true;
        }
}


void get_payment_method() {
        display_returnable_menus("Payment Methods", PAYMENT_METHODS, false);
        int opt = get_numeric_option("Please select a payment method: ", 1, 3);
        ticket.payment_method = PAYMENT_METHODS.at(opt - 1);
        cls();
        display_returnable_menus("Member Types", MEMBER_LIST, false);
        opt = get_numeric_option("Please select your membership: ", 1, 3);
        ticket.member_type = opt - 1;
}


void buy_food() {
        clear_food_items(ticket.foods);
        int food_opt = FOODS.size() + 1, food_quan;
        char cont_food_opt;
        do {
                cls();
                display_food_details();
                display_cart();
                food_opt = get_numeric_option("Select a number to buy: (0 to stop)", 0, FOODS.size());
                if (food_opt > 0) {
                        int food_left = MAX_FOOD_QUAN - ticket.foods.at(food_opt - 1).quantity;
                        if (food_left > 0) {
                                std::string prompt = "Select the quantity to buy (max ";
                                prompt += std::to_string(food_left) + "): ";
                                food_quan = get_numeric_option(prompt, 0, std::min(MAX_FOOD_QUAN, food_left));
                                ticket.foods.at(food_opt - 1).quantity += food_quan;
                        }
                        else {
                                std::cout << "You have bought the maximum amount.\n";
                                prompt_enter();
                                cls();
                        }
                }
                else {
                        cont_food_opt = get_character_option("Are you sure to stop? (Y/N): ", "YyNn");
                        if (std::tolower(cont_food_opt) == 'n') {
                                food_opt = FOODS.size() + 1;
                        }
                }
        } while (food_opt != 0);
}


void get_customer_order() {
        bool cont_order = true;
        char cont_order_opt;
        int buy_ticket_res = 0; //0 = exit normally, 1 = exit with back (without buying food)
        do {
                ticket = {};
                buy_ticket_res = buy_ticket();
                cls();
                if (buy_ticket_res == 0) {
                        cont_order_opt = get_character_option("Would you like to buy food items? (Y/N): ", "YyNn");
                        if (std::tolower(cont_order_opt) == 'y') {
                                buy_food();
                        }
                        cls();
                        get_payment_method();
                        cls();
                        display_receipt();
                        std::cout << "Thank you for your purchase.\n";
                        transfer_seats_to_list(ticket.selected_movie_idx, ticket.selected_date_idx, ticket.selected_time_idx, ticket.seats);
                        prompt_enter();
                        cls();
                        ALL_SALES.back() += today_sales;
                }
                cont_order_opt = get_character_option("Do you want to continue buying a ticket? (Y/N): ", "YyNn");
                if (std::tolower(cont_order_opt) == 'n') {
                        cont_order = false;
                }
        } while (cont_order);


        std::cout << "Returning to main menu.\n";
        prompt_enter();
        cls();
}


//generates an ASCII representation of a bar graph given its sales level (rounded down with a d of 100)
void generate_report(std::vector<double> &daySales, int day, int start_day = 0) {
        std::cout << std::setfill('-') << std::setw((MAX_LENGTH - 14) / 2) << "-" << " Weekly Report " << std::setw((MAX_LENGTH - 14) / 2) << " " << "\n";
        
        bool dayStartDraw[7] = { 0, 0, 0, 0, 0, 0, 0 };
        int maximumRange = 1500;
        int increment = 100;
        int daySalesRounded;


        for (int currentPriceLevel = maximumRange; currentPriceLevel >= 0; currentPriceLevel -= increment) {
                (currentPriceLevel % 100 == 0) ? std::cout << std::setw(4) << currentPriceLevel : std::cout << "    ";
                std::cout << "      ";
                for (int day = 0; day < 7; day++) {
                        daySalesRounded = static_cast<int>(daySales[day] / increment) * increment;
                        if (!dayStartDraw[day]) {
                                if (daySalesRounded == currentPriceLevel) {
                                        if (daySalesRounded == 0) {
                                                std::cout << "____";
                                        }
                                        else {
                                                std::cout << "___ ";
                                        }
                                        dayStartDraw[day] = true;
                                }
                                else if (daySalesRounded > currentPriceLevel) {
                                        std::cout << "| | ";
                                        dayStartDraw[day] = true;
                                }
                                else {
                                        std::cout << "    ";
                                }
                        }
                        else {
                                if (currentPriceLevel == 0) {
                                        std::cout << "| |_";
                                }
                                else {
                                        std::cout << "| | ";
                                }
                        }
                }
                std::cout << "\n";
        }
        std::cout << "          ";
        int d = start_day;
        for (; d < start_day + 7; d++) {
                std::cout << DAYS[d % 7] << " ";
        }
        std::cout << "\n\n";
        
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-\n";
        double mean = 0;
        std::cout << "Day: " << day << " (" << DAYS[(day - 1) % 7] << ")\n";
        std::cout << "Today's Profits                : RM" << std::fixed << std::setprecision(2) << std::showpoint << today_sales << "\n";
        std::cout << "Yesterday's Profits            : RM" << std::fixed << std::setprecision(2) << std::showpoint << daySales.at(day > 1 ? (day < 8 ? day - 2 : 5) : 0) << "\n";


        for (double s : daySales) {
                mean += s;
        }
        mean /= 7;


        std::cout << "Mean for the week (today incl.): RM" << std::fixed << std::setprecision(2) << std::showpoint << mean << "\n";
        std::cout << std::setfill('-') << std::setw(MAX_LENGTH) << "-\n" << "\n" << std::setfill(' ');
}


void start_system(int day) {
        bool exitSystem = false;
        std::vector<double> report_sales;;
        while (!exitSystem) {
                display_logo();
                displayMainMenu();
                
                //print the time (optional)
                //not used since it will not reflect the dates on our project
                /*std::time_t current_time;
                std::time(&current_time);
                char current_time_buffer[26];
                ctime_s(current_time_buffer, sizeof(current_time_buffer), & current_time);
                std::cout << current_time_buffer << "\n";*/


                std::cout << "Day: " << day << "\n";
                std::cout << "What would you like to do today?\n";


                int choice = get_numeric_option("Enter your option here: ", 1, 4);
                switch (choice) {
                case 1: {
                        cls();
                        display_returnable_menus("Now showing:", ALL_MOVIES, false);
                        std::cout << "Prices: Adult (RM 12.00), Child (RM 6.50)\n\n";
                        prompt_enter();
                        cls();
                        break;
                }
                case 2: {
                        get_customer_order();
                        break;
                }
                case 3: {
                        cls();
                        report_sales = {};
                        //reorder the sales to show the most recent days
                        if (ALL_SALES.size() < 8) {
                                for (int d = 0; d < 7; d++) {
                                        if (d < ALL_SALES.size()) {
                                                report_sales.push_back(ALL_SALES.at(d));
                                        }
                                        else {
                                                report_sales.push_back(0);
                                        }
                                }
                        }
                        else {
                                for (int d = 1; d < 8; d++) {
                                        report_sales.insert(report_sales.begin(), ALL_SALES.at(ALL_SALES.size() - d));
                                }
                        }
                        generate_report(report_sales, day, (ALL_SALES.size() > 6 ? ALL_SALES.size() % 7 : 0));
                        prompt_enter();
                        cls();
                        break;
                }
                case 4: {
                        if (std::tolower(get_character_option("Would you really like to end the day? (Y/N): ", "YyNn")) == 'y') {
                                exitSystem = true;
                                std::cout << "Exiting system.\n\n";
                                save_seats();
                                save_sales();
                                break;
                        }
                        cls();
                        }
                }
        }
}


//randomise seats with weights (off = vacant, on = taken)
void randomise_seats(std::vector<bool>& seats, int off_weight = 1, int on_weight = 1) {
        int rand_num;
        for (int i = 0; i < seats.size(); i++) {
                rand_num = std::rand() % (off_weight + on_weight);
                rand_num < off_weight ? seats.at(i) = 0 : seats.at(i) = 1;
        }
}


//saves the seats to seats.bin after encoding
int save_seats() {
        std::string encoded_string = encode_bool(ALL_SEATS);
        std::ofstream seats_ofile(SEATS_FILE, std::ios::binary);


        if (seats_ofile.is_open()) {
                seats_ofile << encoded_string;
                seats_ofile.close();
                std::cout << "Seats successfully saved\n";
        }
        else {
                std::cerr << "Error: Unable to open file for writing. Seats are not saved!\n";
                return 1;
        }
        return 0;
}


//loads the seats back into ALL_SEATS
int load_seats() {
        std::ifstream seats_ifile(SEATS_FILE, std::ios::binary);
        if (!seats_ifile.is_open()) {
                std::cerr << "Error opening the file! Randomising seats...\n";
                return 1;
        }


        char ch;
        std::string res = R"()";
        for (int i = 0; i < 7500 / 8 + 1; i++) {
                seats_ifile.get(ch);
                if (!seats_ifile.eof()) {
                        res += ch;
                }
        }
        ALL_SEATS = decode_bool(res, TOTAL_SEATS * ALL_MOVIES.size() * ALL_DATES.size() * ALL_TIMES.size());


        if (ALL_SEATS.size() == (TOTAL_SEATS * ALL_MOVIES.size() * ALL_DATES.size() * ALL_TIMES.size())) {
                std::cout << "Loading Success!\n";
        }
        else {
                std::cerr << "Content Mismatch! Randomising seats...\n";
                ALL_SEATS.assign(TOTAL_SEATS * ALL_MOVIES.size() * ALL_DATES.size() * ALL_TIMES.size(), false);
                std::cout << ALL_SEATS.size();
                return 1;
        }
        return 0;
}


int save_sales() {
        std::string encoded_string = encode_sales(ALL_SALES);
        std::ofstream sales_ofile(SALES_FILE);


        if (sales_ofile.is_open()) {
                sales_ofile << encoded_string;
                sales_ofile.close();
                std::cout << "Sales successfully saved.\n";
        }
        else {
                std::cerr << "Error: Unable to open file for writing. Sales are not saved!\n";
                return 1;
        }
        return 0;
}


int load_sales() {
        std::ifstream sales_ifile(SALES_FILE);
        if (!sales_ifile.is_open()) {
                std::cerr << "Error opening the file! Starting with no sales...\n";
                return 1;
        }


        std::ostringstream res;
        res << sales_ifile.rdbuf();
        ALL_SALES = decode_sales(res.str());
        return 0;
}


int main(int argc, char* argv[]) {
        if (load_seats()) {
                randomise_seats(ALL_SEATS, 1, 1);
        }
        load_sales();
        ALL_SALES.push_back(today_sales); //push back today's sales so it can be seen in the weekly report
        cls();
        start_system(ALL_SALES.size());
        return 0;
}