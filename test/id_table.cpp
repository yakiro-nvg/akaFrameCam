/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <vector>
#include <catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/state.h>
#include <source/id_table.h>

using namespace std;
using namespace akaFrame::cam;
using namespace akaFrame::cam::id_table;

struct IdModel
{
        IdModel(bool in_use, uintptr_t address)
                : in_use(in_use)
                , address(address)
        {
                // nop
        }

        bool in_use;
        uintptr_t address;
};

struct IdTableModel
{
        vector<IdModel> ids;
};

struct IdTableSut
{
        vector<cam_address_t> mapping;
        IdTable tbl;
};

static void full_check(const IdTableModel &model, IdTableSut &sut)
{
        REQUIRE(model.ids.size() == sut.mapping.size());
        for (int i = 0; i < model.ids.size(); ++i) {
                auto mid = model.ids[i];
                auto sid = sut.mapping[i];
                if (mid.in_use) {
                        REQUIRE(resolve<void>(sut.tbl, sid) == (void*)mid.address);
                } else {
                        REQUIRE(resolve<void>(sut.tbl, sid) == nullptr);
                }
        }
}

struct MakeIdCommand
        : rc::state::Command<IdTableModel, IdTableSut>
{
        vector<uintptr_t> addresses;

        MakeIdCommand()
                : addresses(*rc::gen::container<vector<uintptr_t>>(
                        *rc::gen::inRange<size_t>(1, 512),
                        rc::gen::nonZero<uintptr_t>()))
        {
                // nop
        }

        void apply(Model &s0) const override
        {
                for (int i = 0; i < (int)addresses.size(); ++i) {
                        s0.ids.emplace_back(true, addresses[i]);
                }
        }

        void run(const Model &s0, Sut &sut) const override
        {
                full_check(s0, sut);
                for (int i = 0; i < (int)addresses.size(); ++i) {
                        const auto id = make(sut.tbl, (void*)addresses[i]);
                        sut.mapping.push_back(id);
                }
        }
};

struct InUseIndexCommandBase
        : rc::state::Command<IdTableModel, IdTableSut>
{
        int32_t random;

        inline int32_t index(const Model &s0) const
        {
                // clamp index to size of ids
                return random%s0.ids.size();
        }

        InUseIndexCommandBase()
                : random(*rc::gen::arbitrary<int32_t>())
        {
                // nop
        }

        void checkPreconditions(const Model &s0) const override
        {
                RC_PRE(s0.ids.size() > 0);
                RC_PRE(s0.ids[index(s0)].in_use);
        }
};

struct DropIdCommand
        : public InUseIndexCommandBase
{
        void apply(Model &s0) const override
        {
                auto &mid = s0.ids[index(s0)];
                mid.in_use = false;
        }

        void run(const Model &s0, Sut &sut) const override
        {
                full_check(s0, sut);
                const auto sid = sut.mapping[index(s0)];
                drop(sut.tbl, sid);
        }
};

struct RelocateCommand
        : public InUseIndexCommandBase
{
        uintptr_t address;

        RelocateCommand()
                : address(*rc::gen::nonZero<uintptr_t>())
        {
                // nop
        }

        void apply(Model &s0) const override
        {
                auto &mid = s0.ids[index(s0)];
                mid.address = address;
        }

        void run(const Model &s0, Sut &sut) const override
        {
                full_check(s0, sut);
                auto &mid = s0.ids[index(s0)];
                const auto sid = sut.mapping[index(s0)];
                REQUIRE((void*)mid.address == relocate(sut.tbl, sid, (void*)address));
        }
};

struct ClearCommand
        : rc::state::Command<IdTableModel, IdTableSut>
{
        void apply(Model &s0) const override
        {
                for (int i = 0; i < (int)s0.ids.size(); ++i) {
                        s0.ids[i].in_use = false;
                }
        }

        void run(const Model &s0, Sut &sut) const override
        {
                full_check(s0, sut);
                for (int i = 0; i < (int)s0.ids.size(); ++i) {
                        if (s0.ids[i].in_use) {
                                drop(sut.tbl, sut.mapping[i]);
                        }
                }
        }
};

TEST_CASE("id_table")
{
        rc::check([&]() {
                IdTableModel initial_state;
                IdTableSut sut;

                rc::state::check(initial_state, sut, rc::state::gen::execOneOfWithArgs<
                        MakeIdCommand, DropIdCommand, RelocateCommand, ClearCommand>());
        });
}
