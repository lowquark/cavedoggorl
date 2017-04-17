
math.randomseed(os.time())

map_size(40, 40)

for i=0,40 do
  for j=0,40 do
    load_tile(i, j, 0)
  end
end

for b=1,10 do
  local w = math.random(5, 10)
  local h = math.random(5, 10)
  local x = math.random(0, 40 - w)
  local y = math.random(0, 40 - h)

  for i=0,w-1 do
    for j=0,h-1 do
      load_tile(x + i, y + j, 1);
    end
  end
end

place_player(20, 20)
for m=1,4 do
  place_impostor(math.random(0, 40-1), math.random(0, 40-1))
end

