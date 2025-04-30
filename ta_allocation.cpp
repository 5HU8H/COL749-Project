#include <bits/stdc++.h>
#include "matchings.h"


struct Student {
    int M_sid;
    double M_cgpa;
    std::string M_sname;
    std::vector<std::pair<int, int>> M_pref;  // (cid, grade_in_cid)

    auto get_id() const {
        return M_sid;
    }

    auto get_cgpa() const {
        return M_cgpa;
    }

    auto get_name() const {
        return M_sname;
    }

    const auto& get_pref() const {
        return M_pref;
    }
};


struct Course {
    int M_cid;
    int M_vacancy;
    std::string M_cname;

    virtual bool better(const Student &s1, const Student &s2) const = 0;

    auto get_vacancy() const {
        return M_vacancy;
    }

    auto get_name() const {
        return M_cname;
    }

    auto get_id() const {
        return M_cid;
    }
};

// Pref = callable(const Course&, const Student& s1, const Student& s2)
template <typename _Pref>
struct CourseCompare : public Course {
    _Pref M_pref;

    CourseCompare(std::string_view _cname, int _cid, int _vacancy, _Pref &_pref) : M_cname(_cname), M_cid(_cid), M_vacancy(_vacancy), M_pref(_pref) 
    { 
        assert(M_vacancy >= 1);
    }

    bool better(const Student &s1, const Student &s2) const override {
        return M_pref(*this, s1, s2);
    }
};

// returns (sid, cid) pairs
std::vector<std::pair<int, int>> 
get_allotment(std::vector<Course> &courses, std::vector<Student> &students) {
    std::sort(courses.begin(), courses.end(), [](const auto &c1, const auto &c2) {
        return c1.get_id() < c2.get_id();
    });
    std::sort(students.begin(), students.end(), [](const auto &s1, const auto &s2) {
        return s1.get_id() < s2.get_id();
    });

    int n = students.size();
    int m = n;
    std::vector<int> cstart(courses.size());
    for (int i = 0; i < courses.size(); i++) {
        cstart[i] = m;
        m += courses[i].get_vacancy();
    }
    m -= n;

    auto SIdIndex = [&students](int sid) -> int {
        return std::lower_bound(students.begin(), students.end(), sid, [](const Student &s, int id) {
            return s.get_id() < id;
        }) - students.begin();
    };
    auto CIdIndex = [&courses](int cid) -> int {
        return std::lower_bound(courses.begin(), courses.end(), cid, [](const Course &c, int id) {
            return c.get_id() < id;
        }) - courses.begin();
    };

    std::vector<std::vector<int>> g(n + m);

    for (int i = 0; i < n; i++) {
        const auto& pref = students[i].get_pref();
        for (int j = 0; j < pref.size(); j++) {
            int idx = CIdIndex(pref[j].first);
            g[n + idx].push_back(i);
            const auto &course = courses[idx];
            for (int k = 0; k < course.get_vacancy(); k++) {
                g[i].push_back(cstart[idx] + k);
            }
        }
    }

    for (int i = std::ssize(courses) - 1; i >= 0; i--) {
        const auto &course = courses[i];
        std::sort(g[i + n].begin(), g[i + n].end(), [&course, &students](int i1, int i2) {
            return course.better(students[i1], students[i2]);
        });
        for (int j = cstart[i]; j < cstart[i] + course.get_vacancy(); j++) {
            g[j + n] = g[i + n];
        }
    }

    auto mt = max_cardinality_least_unpopular(g, n, m);
    std::vector<std::pair<int, int>> allotment;
    for (int i = 0; i < n; i++) {
        if (mt[i] != -1) {
            auto j = mt[i] - n;
            j = std::lower_bound(cstart.begin(), cstart.end(), j) - cstart.begin();
            allotment.emplace_back(students[i].get_id(), courses[j].get_id());
        }
    }

    return allotment;
}