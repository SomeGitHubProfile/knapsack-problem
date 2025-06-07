#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>

using namespace std;

struct Item {
    int price;
    int weight;
};

// структура состояния
struct State {
private:
    double calculate_upper_boundary(int max_weight, const vector<Item>& items) noexcept {
        if (max_weight <= weight) { // если рюкзак полон
            return price; // то это максимальная стоимость
        }

        double upper_value = price;
        int weight_left = max_weight - weight; // оставшийся вес (сколько еще можем набрать до переполнения)

        // жадный алгоритм
        for (int i = index + 1; i < items.size(); ++i) {
            if (items[i].weight <= weight_left) {
                upper_value += items[i].price; // добавляем предмет
                weight_left -= items[i].weight; // уменьшаем оставшийся допустимый вес
            } else {
                double specific_price = static_cast<double>(items[i].price) / items[i].weight; // удельная стоимость
                return upper_value + weight_left * specific_price; // добавляем "часть" предмета, исходя из удельной стоимости, и больше ничего добавить нельзя
            }
        }
        return upper_value;
    }
public:
    int index; // индекс предмета
    int price; // уже набранная стоимость
    int weight; // уже набранный вес
    double upper_boundary; // верхняя оценка на стоимость

    State(int max_weight, const vector<Item>& items, int state_index, int state_price, int state_weight) noexcept :
        index(state_index),
        price(state_price),
        weight(state_weight),
        upper_boundary(calculate_upper_boundary(max_weight, items)) {}

    bool operator<(const State& rhs) const noexcept {
        return upper_boundary < rhs.upper_boundary;
    }
};

int solve(int max_weight, const vector<Item>& items) noexcept {
    vector<Item> sorted_items = items;
    sort(sorted_items.begin(), sorted_items.end(), [](const Item& lhs, const Item& rhs) {
        double lhs_specific_price = static_cast<double>(lhs.price) / lhs.weight; // удельная стоимость lhs
        double rhs_specific_price = static_cast<double>(rhs.price) / rhs.weight; // удельная стоимость rhs
        return rhs_specific_price < lhs_specific_price; // сортируем по убыванию удельной стоимости
    });

    auto create_state = [&](int index, int price, int weight) {
        return State(max_weight, sorted_items, index, price, weight);
    };

    priority_queue<State> queue;
    State root = create_state(-1, 0, 0);
    queue.push(root);
    int best_price = 0;
    while (!queue.empty()) {
        State current_state = queue.top();
        queue.pop();

        if (current_state.upper_boundary < best_price) {
            continue; // точно не наберем цену лучше
        }

        if (current_state.index == items.size() - 1) { // если обработали последний предмет
            best_price = max(best_price, current_state.price); // обновляем лучшую цену
            continue;
        }

        int next_index = current_state.index + 1;
        State without_next_item = create_state(next_index, current_state.price, current_state.weight);
        State with_next_item = create_state(next_index, current_state.price + items[next_index].price, current_state.weight + items[next_index].weight);

        if (best_price <= without_next_item.upper_boundary) {
            queue.push(without_next_item);
        }

        if (best_price <= with_next_item.upper_boundary && with_next_item.weight <= max_weight) {
            queue.push(with_next_item);
        }
    }
    return best_price;
}

vector<Item> manual_input(int& n, int& max_weight) noexcept {
    cout << "Input number of items: ";
    cin >> n;
    cout << "Input max backpack weight: ";
    cin >> max_weight;
    cout << "Input n pairs: price - weight\n";
    vector<Item> items(n);
    for (int i = 0; i < n; ++i) {
        cin >> items[i].price >> items[i].weight;
    }
    return items;
}

vector<Item> file_input(int& n, int& max_weight, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: can't open the file '" << filename << "'.\n";
        return {};
    }

    file >> n >> max_weight;
    vector<Item> items(n);
    for (int i = 0; i < n; ++i) {
        file >> items[i].price >> items[i].weight;
    }
    return items;
}

vector<Item> input_data(int& n, int& max_weight) {
    string filename;
    cout << "Enter input filename or press Enter to manually input data: ";
    getline(cin, filename);
    if (filename.empty()) {
        return manual_input(n, max_weight);
    }
    return file_input(n, max_weight, filename);
}

int main() {
    int n, max_weight;
    vector<Item> items = input_data(n, max_weight);
    auto start = chrono::high_resolution_clock::now();
    int best_price = solve(max_weight, items);
    auto end = chrono::high_resolution_clock::now();
    auto time_spent = end - start;
    cout << "Maximum price that can be taken: " << best_price << '\n';
    cout << "Time spent:\n";
    cout << "t = " << time_spent.count() << " nanoseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::milliseconds>(time_spent).count() << " milliseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::seconds>(time_spent).count() << " seconds\n";
#if defined(NDEBUG)
    std::cout << "Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();
#endif
    return 0;
}