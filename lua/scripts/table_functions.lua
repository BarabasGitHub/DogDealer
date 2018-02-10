-- only copies the first order elements in the table, any nested tables are not copied
function ShallowCopyTable(table)
  local copy = {}
  for key, value in pairs(table) do
    copy[key] = value
  end
  return copy
end


-- does a shallow merge (only copies first level elements) of the two tables and returns a new table
-- overwrites all values in table1 with the values in table2
function MergeTables( table1, table2 )
    local output = ShallowCopyTable(table1)
    for key,value in pairs(table2) do
        output[key] = value
    end
    return output
end