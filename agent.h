/**
 * Framework for Threes! and its variants (C++ 11)
 * agent.h: Define the behavior of variants of agents including players and
 * environments
 *
 * Author: Theory of Computer Games
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <algorithm>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

#include "action.h"
#include "board.h"
#include "weight.h"

class agent {
   public:
    agent(const std::string& args = "") {
        std::stringstream ss("name=unknown role=unknown " + args);
        for (std::string pair; ss >> pair;) {
            std::string key = pair.substr(0, pair.find('='));
            std::string value = pair.substr(pair.find('=') + 1);
            meta[key] = {value};
        }
    }
    virtual ~agent() {}
    virtual void open_episode(const std::string& flag = "") {}
    virtual void close_episode(const std::string& flag = "") {}
    virtual action take_action(const board& b) { return action(); }
    virtual bool check_for_win(const board& b) { return false; }

   public:
    virtual std::string property(const std::string& key) const {
        return meta.at(key);
    }
    virtual void notify(const std::string& msg) {
        meta[msg.substr(0, msg.find('='))] = {msg.substr(msg.find('=') + 1)};
    }
    virtual std::string name() const { return property("name"); }
    virtual std::string role() const { return property("role"); }

   protected:
    typedef std::string key;
    struct value {
        std::string value;
        operator std::string() const { return value; }
        template <typename numeric,
                  typename = typename std::enable_if<
                      std::is_arithmetic<numeric>::value, numeric>::type>
        operator numeric() const {
            return numeric(std::stod(value));
        }
    };
    std::map<key, value> meta;
};

/**
 * base agent for agents with randomness
 */
class random_agent : public agent {
   public:
    random_agent(const std::string& args = "") : agent(args) {
        if (meta.find("seed") != meta.end()) engine.seed(int(meta["seed"]));
    }
    virtual ~random_agent() {}

   protected:
    std::default_random_engine engine;
};

/**
 * base agent for agents with weight tables and a learning rate
 */
class weight_agent : public agent {
   public:
    weight_agent(const std::string& args = "") : agent(args), alpha(0.1f) {
        // if (meta.find("init") != meta.end()) init_weights(meta["init"]);
        // if (meta.find("load") != meta.end()) load_weights(meta["load"]);
        // if (meta.find("alpha") != meta.end()) alpha = float(meta["alpha"]);
    }
    virtual ~weight_agent() {
        if (meta.find("save") != meta.end()) save_weights(meta["save"]);
    }

   protected:
    virtual void init_weights(const std::string& info) {
        std::string res = info;  // comma-separated sizes, e.g., "65536,65536"
        for (char& ch : res)
            if (!std::isdigit(ch)) ch = ' ';
        std::stringstream in(res);
        // for (size_t size; in >> size; net.emplace_back(size));
        std::cout<<"weight_agent init_weights"<<std::endl;
    }
    virtual void load_weights(const std::string& path) {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in.is_open()) std::exit(-1);
        uint32_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        net.resize(size);
        for (weight& w : net) {
            in >> w;
        }
        in.close();
        std::cout<<"load weights from "<<path<<std::endl;
    }
    virtual void save_weights(const std::string& path) {
        std::ofstream out(path,
                          std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) std::exit(-1);
        uint32_t size = net.size();
        out.write(reinterpret_cast<char*>(&size), sizeof(size));
        for (weight& w : net) {
            out << w;
        }
        out.close();
        std::cout<<"save weights to "<<path<<std::endl;
    }

   protected:
    std::vector<weight> net;
    float alpha;
};

/*
* time difference learning agent
*/
class tdl_slider : public weight_agent {
   public:
    tdl_slider(const std::string& args = "")
        : weight_agent("name=tdl role=player " + args), opcode({0, 1, 2, 3}) {
        state_path.reserve(20000000);
        if (meta.find("init") != meta.end()) init_weights(meta["init"]);
        if (meta.find("load") != meta.end()){
            init_weights(meta["init"]);
            load_weights(meta["load"]);
        }
        if (meta.find("alpha") != meta.end()) alpha = float(meta["alpha"]);
    }

    virtual void init_weights(const std::string& info) {
        net.emplace_back(weight({0, 1, 2, 3, 4, 5}));
        net.emplace_back(weight({4, 5, 6, 7, 8, 9}));
        net.emplace_back(weight({0, 1, 2, 4, 5, 6}));
        net.emplace_back(weight({4, 5, 6, 8, 9, 10}));
        std::cout<<"init_weights"<<std::endl;
    }

    virtual action take_action(const board& before ) {
        std::vector<board> after_boards;
        std::vector<board::reward> after_rewards;

        int best_op_idx = -1;
		board::reward best_reward = -1;
        for ( size_t op_idx = 0 ; op_idx < opcode.size(); op_idx++) {
			int op = opcode[op_idx];
			after_boards.emplace_back(board(before));
            board::reward reward = after_boards[op_idx].slide(op);
            board::reward est_reward;

            after_rewards.emplace_back(reward);

            //keep unavailable moves -1
            if( reward != -1 ) {
                est_reward = estimate(after_boards[op_idx]);
                //TODO: check if this is correct
                // if( est_reward > 0 )
                //     std::cout<<"est_reward: "<<est_reward<<std::endl;
            }
            else {
                est_reward = -1;
            }

            if ( reward + est_reward > best_reward) {
                best_reward = reward + est_reward;
                best_op_idx = op_idx;
            }
        }
        if ( best_reward != -1 ) {
			state cur_state = {};
            cur_state.before = before;
            cur_state.after = after_boards[best_op_idx];
            cur_state.op = opcode[best_op_idx];
            cur_state.reward = after_rewards[best_op_idx];
            cur_state.value = best_reward;

            state_path.emplace_back(cur_state);
            return action::slide(opcode[best_op_idx]);
        }
        state_path.emplace_back(state());
        return action();
    }

    void update_episode() {
        float exact = 0;
        for (state_path.pop_back(); state_path.size(); state_path.pop_back()) {
            state& move = state_path.back();
            float error = exact - (move.value - move.reward);
            exact = move.reward + update(move.after, alpha * error);
        }
        state_path.clear();
    }
    /**
     * accumulate the total value of given state
     */
    float estimate(const board& b) const {
        float value = 0;
        for (auto& p : net) {
            value += p.estimate(b);
        }
        return value;
    }

    /**
     * update the value of given state and return its new value
     */
    float update(const board& b, float u) {
        float u_split = u / net.size();
        // if( u > 0 )
        //     std::cout<<"update: "<<u_split<<std::endl;
        float value = 0;
        for (auto& p : net) {
            value += p.update(b, u_split);
        }
        return value;
    }

   private:
    struct state {
        board before, after;
        int op;
        board::reward reward, value;
    };
    std::vector<state> state_path;
    std::array<int, 4> opcode;
};



/**
 * default random environment, i.e., placer
 * place the hint tile and decide a new hint tile
 */
class random_placer : public random_agent {
   public:
    random_placer(const std::string& args = "")
        : random_agent("name=place role=placer " + args) {
        spaces[0] = {12, 13, 14, 15};
        spaces[1] = {0, 4, 8, 12};
        spaces[2] = {0, 1, 2, 3};
        spaces[3] = {3, 7, 11, 15};
        spaces[4] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    }

    virtual action take_action(const board& after) {
        std::vector<int> space = spaces[after.last()];
        std::shuffle(space.begin(), space.end(), engine);
        for (int pos : space) {
            if (after(pos) != 0) continue;

            int bag[3], num = 0;
            for (board::cell t = 1; t <= 3; t++)
                for (size_t i = 0; i < after.bag(t); i++) bag[num++] = t;
            std::shuffle(bag, bag + num, engine);

            board::cell tile = after.hint() ?: bag[--num];
            board::cell hint = bag[--num];

            return action::place(pos, tile, hint);
        }
        return action();
    }

   private:
    std::vector<int> spaces[5];
};

/**
 * random player, i.e., slider
 * select a legal action randomly
 */
class random_slider : public random_agent {
   public:
    random_slider(const std::string& args = "")
        : random_agent("name=slide role=slider " + args),
          opcode({0, 1, 2, 3}) {}

    virtual action take_action(const board& before) {
        std::shuffle(opcode.begin(), opcode.end(), engine);
        for (int op : opcode) {
            board::reward reward = board(before).slide(op);
            if (reward != -1) return action::slide(op);
        }
        return action();
    }

   private:
    std::array<int, 4> opcode;
};

/**
 * heuristic player, i.e., slider
 * select a legal action based on heuristic
 */
class heuristic_slider : public random_agent {
   public:
    heuristic_slider(const std::string& args = "")
        : random_agent("name=slide role=slider " + args), opcode({0, 2, 3}) {}

    virtual action take_action(const board& before) {
        int best_op = -1, best_reward = -1;
        for (int op : opcode) {
            // filter out ops  to prevent messing up with the corner val
            // if( op == 1 && !board(before).top_row_right_stuck() ) continue;
            // if( op == 2 && !board(before).left_col_down_stuck() ) continue;

            board::reward reward = board(before).slide(op);
            if (reward > best_reward) {
                best_reward = reward;
                best_op = op;
            }
        }
        if (best_op != -1) {
            return action::slide(best_op);
        }
        return action();
    }

   private:
    std::array<int, 4> opcode;
};
