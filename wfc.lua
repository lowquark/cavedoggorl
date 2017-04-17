#!/usr/bin/luajit

function num_possible(st)
  local sum = 0
  for i,p in ipairs(st) do
    if p then sum = sum + 1 end
  end
  return sum
end

function choose(st)
  local n = num_possible(st)

  if n == 0 then error('can\'t choose!') end

  local r = math.random(1, n)
  for i,p in ipairs(st) do
    if p then r = r - 1 end
    if r == 0 then
      return i
    end
  end

  error('couldn\'t choose?!?')
end

function entropy_heuristic(st)
  local sum = 0
  for i,p in ipairs(st) do
    if p then sum = sum + 1 end
  end
  return sum
end


math.randomseed(os.time())

local out_size = 10

local state_ps = {}
local states = {}

for i=1,out_size do
  state_ps[i] = { true, true, true }
end


while true do
  local min_ps = nil
  local min_idx = 0
  local min_entropy = math.huge

  for i,ps in ipairs(state_ps) do
    if not states[i] then
      local entropy = entropy_heuristic(ps)

      if entropy < min_entropy then
        min_ps = ps
        min_idx = i
        min_entropy = entropy
      end
    end
  end

  if min_ps then
    local new_state = choose(min_ps)
    states[min_idx] = new_state

    -- wave function:
    -- 1 1 0 0
    --
    -- N = 2
    --
    -- states:
    -- 1: 1 1 (x1)
    -- 2: 1 0 (x1)
    -- 3: 0 0 (x1)
    --
    -- contiguity:
    -- 1 -> 1
    -- 1 -> 2
    -- 2 -> 3
    -- 3 -> 3
    --
    -- ~contiguity:
    -- 1 -> 3
    -- 2 -> 1
    -- 2 -> 2
    -- 3 -> 1
    -- 3 -> 2

    -- need to propogate impossible states

    -- check if we have a neighbor
    local neighbor_idx = min_idx + 1
    if neighbor_idx <= out_size then
      -- follow ~contiguity table
      if new_state == 1 then
        state_ps[neighbor_idx][3] = false
      elseif new_state == 2 then
        state_ps[neighbor_idx][1] = false
        state_ps[neighbor_idx][2] = false
      elseif new_state == 3 then
        state_ps[neighbor_idx][1] = false
        state_ps[neighbor_idx][2] = false
      end
    end
  else
    break
  end
end

for i,st in ipairs(states) do
  print(i, st)
end

