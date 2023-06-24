#include <iostream>


#include<vector>
#include <queue>
#include <set>

using namespace std;

class Solution {
public:
    int firstMissingPositive(vector<int> nums) {
        for (int i = 0; i < nums.size(); ++i) {
            while (nums[i] - 1 < nums.size() && nums[i] > 0 && nums[i] != i + 1) {
                std::swap(nums[i], nums[nums[i] - 1]);
            }
        }
        for (int i = 0; i < nums.size(); ++i) {
            if (nums[i] != i + 1) {
                return i + 1;
            }
        }
        return static_cast<int>(nums.size() + 1);
    }

    vector<int> maxSlidingWindow(vector<int> nums, int k) {
        vector<int> res(nums.size() - k + 1);
        std::deque<int> q;
        for (int i = 0; i < k; ++i) {
            while (!q.empty() && nums[q.back()] < nums[k]) {
                q.pop_back();
            }
            q.push_back(i);
        }
        for (int i = k; i < nums.size(); ++i) {
            res[i - k] = nums[q.front()];
            while (!q.empty() && nums[q.back()] < nums[i]) {
                q.pop_back();
            }
            while (!q.empty() && q.back() <= i - k) {
                q.pop_front();
            }
            q.push_back(i);
        }
        if (!q.empty())
            res.push_back(q.front());
        return res;
    }
};

int main() {
    Solution s;
    s.maxSlidingWindow({1, -1}, 1);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
