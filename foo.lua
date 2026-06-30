-- example retromap script
--
-- e.g.
-- $ retromap --script foo.lua mapfile.r2
--
--
-- Script environment has std lua libs.
-- Also has `proj`, a table-based dump of the loaded map project.


function dump(v)
    if indent == nil then
        indent = "  "
    end
    if type(v) ~= 'table' then
        return tostring(v)
    end
    local parts = {}
    for key, value in pairs(v) do
        parts[#parts+1] = string.format("%s: %s", key, dump(value))
    end
    return '{' .. table.concat(parts, ", ") .. '}'
end


print(dump(proj))

