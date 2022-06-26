#include <clang-c/Index.h>

#include <string>

void visitChildrenOfKind(CXCursor cursor, CXCursorKind kind, std::function<void(CXCursor)> action)
{
    struct ClientData
    {
        CXCursorKind kind;
        std::function<void(CXCursor)> action;
    };
    ClientData data{kind, action};
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data)
        {
            auto data = static_cast<ClientData *>(client_data);
            if (clang_getCursorKind(child) == data->kind)
            {
                data->action(child);
            }
            return CXChildVisit_Continue;
        },
        &data);
}

template <typename Result>
std::vector<Result> mapChildrenOfKind(CXCursor cursor, CXCursorKind kind, std::function<Result(CXCursor)> converter)
{
    std::vector<Result> results;
    visitChildrenOfKind(cursor, kind, [&results, converter](CXCursor child)
                        { results.push_back(converter(child)); });
    return results;
}

template <typename Result>
std::vector<Result> mapChildrenOfKindIf(CXCursor cursor, CXCursorKind kind, std::function<bool(CXCursor)> condition, std::function<Result(CXCursor)> converter)
{
    std::vector<Result> results;
    visitChildrenOfKind(cursor, kind, [&results, condition, converter](CXCursor child)
                        { if (condition(child)) results.push_back(converter(child)); });
    return results;
}

template <CXCursorKind kind, typename Result>
std::vector<Result> mapChildren(CXCursor cursor)
{
    return mapChildrenOfKind<Result>(cursor, kind, [](CXCursor child)
                                     { return Result(child); });
}

template <CXCursorKind kind, typename Result>
std::vector<Result> mapChildrenIf(CXCursor cursor, std::function<bool(CXCursor)> condition)
{
    return mapChildrenOfKindIf<Result>(cursor, kind, condition, [](CXCursor child)
                                     { return Result(child); });
}
