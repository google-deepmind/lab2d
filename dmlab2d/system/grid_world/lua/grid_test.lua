--[[ Copyright (C) 2019 The DMLab2D Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local grid_world = require 'system.grid_world'
local random = require 'system.random'

local mocking = require 'testing.mocking'
local mock = mocking.mock
local when = mocking.when
local verify = mocking.verify
local capture = mocking.capture
local verifyNoMoreInteractions = mocking.verifyNoMoreInteractions

local TEST_WORLD = {
    world = grid_world.World{
        renderOrder = {'layer0', 'layer2'},
        updateOrder = {'phase0', {name = 'phase1', func = 'funcPhase1'}},
        types = {
            type0 = {
                layer = 'layer0',
                sprite = '0sprite',
                groups = {'type0', 'type0or1', 'all'},
            },
            type1 = {
                layer = 'layer1',
                sprite = '1sprite',
                groups = {'type1', 'type0or1', 'all'},
            },
            type2 = {
                layer = 'layer2',
                sprite = '2sprite',
                groups = {'type2', 'all'},
                contact = "contact2",
            },
            typeOffGrid = {}
        }
    },
    stateMap = {
        ['0'] = 'type0',
        ['1'] = 'type1',
        ['2'] = 'type2',
    }
}


local tests = {}

function tests.canToString()
  local layout = '001122\n' ..
                 '112233\n' ..
                 '010203\n'
  local grid = TEST_WORLD.world:createGrid{
      layout = layout,
      stateMap = TEST_WORLD.stateMap
  }
  asserts.EQ(tostring(grid), '00  22\n' ..
                             '  22  \n' ..
                             '0 020 \n')
end

function tests.canCreateGrid()
  local grid, pieces = TEST_WORLD.world:createGrid{
      layout = '0 1 2',
      stateMap = TEST_WORLD.stateMap
  }
  asserts.EQ(tostring(grid), '0   2\n')
  asserts.EQ(#pieces, 3)
  asserts.EQ(grid:state(pieces[1]), 'type0')
  asserts.EQ(grid:state(pieces[2]), 'type1')
  asserts.EQ(grid:state(pieces[3]), 'type2')
end


function tests.canDestroyGrid()
  local grid, pieces = TEST_WORLD.world:createGrid{
      layout = '0 1 2',
      stateMap = TEST_WORLD.stateMap
  }
  grid:destroy()
  asserts.shouldFail(function() grid:destroy() end)
end

function tests.canCreateGridWithSize()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  asserts.EQ(tostring(grid), '     \n')
end

function tests.canCreateLayout()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  asserts.EQ(tostring(grid), '     \n')
  local pieces = grid:createLayout{
      layout = '0 1 2',
      stateMap = TEST_WORLD.stateMap,
  }
  asserts.EQ(tostring(grid), '0   2\n')
  asserts.EQ(#pieces, 3)
  asserts.EQ(grid:state(pieces[1]), 'type0')
  asserts.EQ(grid:layer(pieces[1]), 'layer0')
  asserts.EQ(grid:state(pieces[2]), 'type1')
  asserts.EQ(grid:layer(pieces[2]), 'layer1')
  asserts.EQ(grid:state(pieces[3]), 'type2')
  asserts.EQ(grid:layer(pieces[3]), 'layer2')
end

function tests.canCreateGridWithSize()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  asserts.EQ(tostring(grid), '     \n')
end

function tests.canCreateLayout()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  asserts.EQ(tostring(grid), '     \n')
  local pieces = grid:createLayout{
      layout = '0 1 2',
      stateMap = TEST_WORLD.stateMap,
  }
  asserts.EQ(tostring(grid), '0   2\n')
  asserts.EQ(#pieces, 3)
  asserts.EQ(grid:state(pieces[1]), 'type0')
  asserts.EQ(grid:state(pieces[2]), 'type1')
  asserts.EQ(grid:state(pieces[3]), 'type2')
end

function tests.canTransitionStates()
  local types = {}
  local callbacks = {}
  for i = 0, 9 do
    local state = tostring(i)
    types[state] = {layer = '0', sprite = state}

    local nextState = i < 9 and tostring(i + 1) or nil
    callbacks[state] = {
      onAdd = function(grid, piece)
        if nextState then
          grid:setState(piece, nextState)
        end
      end
    }
  end
  local world = grid_world.World{renderOrder = {'0'}, types = types}
  local grid = world:createGrid{
    layout = ' ',
    stateMap = {},
    stateCallbacks = callbacks
  }
  local piece = grid:createPiece('0', {pos = {0, 0}, orientation = 'N'})
  asserts.EQ(tostring(grid), '0\n')
  grid:update(random)
  asserts.EQ(tostring(grid), '9\n')
  grid:setState(piece, '0')
  grid:update(random, 0)
  asserts.EQ(tostring(grid), '0\n')
  grid:update(random, 0)
  asserts.EQ(tostring(grid), '1\n')
  grid:update(random, 2)
  asserts.EQ(tostring(grid), '4\n')
end

function tests.canRemoveInQueue()
  local types = {}
  local callbacks = {}
  for i = 0, 9 do
    local state = tostring(i)
    types[state] = {layer = '0', sprite = state}
    if i < 9 then
      local nextState = tostring(i + 1)
      callbacks[state] = {
          onAdd = function(grid, piece) grid:setState(piece, nextState) end
      }
    end
    if i == 4 then
      callbacks[state].onRemove = function(grid, piece)
        grid:removePiece(piece)
      end
    end
  end
  local world = grid_world.World{renderOrder = {'0'}, types = types}
  local grid = world:createGrid{
    layout = ' ',
    stateMap = {},
    stateCallbacks = callbacks
  }
  local piece = grid:createPiece('0', {pos = {0, 0}, orientation = 'N'})
  asserts.EQ(tostring(grid), '0\n')
  grid:update(random, 1)
  asserts.EQ(tostring(grid), '2\n')
  grid:removePiece(piece)
  asserts.EQ(tostring(grid), ' \n')
  local piece = grid:createPiece('4', {pos = {0, 0}, orientation = 'N'})
  asserts.EQ(tostring(grid), '4\n')
  grid:update(random, 0)
  asserts.EQ(tostring(grid), ' \n')
end

function tests.canCreateWithoutLayer()
  local world = grid_world.World{renderOrder = {}, types = {['0'] = {}}}
  local callbacks = mock()
  when(callbacks).onBlocked(capture.dotDotDot())
  local grid = world:createGrid{
      layout = '  ', stateMap = {},
      stateCallbacks = {['0'] = callbacks}
  }
  local piece0 = grid:createPiece('0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('0', {pos = {1, 0}, orientation = 'N'})

  asserts.EQ(grid:layer(piece0), nil)
  asserts.EQ(grid:layer(piece1), nil)

  asserts.tablesEQ(grid:transform(piece1), {pos = {1, 0}, orientation = 'N'})
  grid:moveAbs(piece1, 'W')
  grid:update(random)
  asserts.tablesEQ(grid:transform(piece1), {pos = {0, 0}, orientation = 'N'})
  grid:moveAbs(piece1, 'W')
  grid:update(random)
  verify(callbacks).onBlocked(grid, piece1, nil)
  asserts.tablesEQ(grid:transform(piece1), {pos = {0, 0}, orientation = 'N'})
end

function tests.createGridErrorOnMissingTable()
  asserts.shouldFail(function() TEST_WORLD.world:createGrid() end, 'table')
end

function tests.createGridErrorOnMissingLayout()
  asserts.shouldFail(
      function()
        TEST_WORLD.world:createGrid{stateMap = TEST_WORLD.stateMap}
      end,
      'layout')
end

function tests.createGridErrorOnMissingTypeMap()
  asserts.shouldFail(
    function() TEST_WORLD.world:createGrid{layout = ' '} end,
    'stateMap')
end

function tests.createGridErrorOnInvalidTypeMap()
  asserts.shouldFail(
      function()
        TEST_WORLD.world:createGrid{
            layout = ' ',
            stateMap = {longKey = 'type0'}}
      end,
      'longKey')
end

function tests.createGridErrorOnMissingType()
  asserts.shouldFail(
      function()
        TEST_WORLD.world:createGrid{
            layout = ' ',
            stateMap = {a = 'typeMissing'}
        }
      end,
      'typeMissing')
end

function tests.canCreateGridJaggedText()
  local layout = '\n\n' ..
                 '22\n' ..
                 '2222\n' ..
                 '222\n\n'
  local grid = TEST_WORLD.world:createGrid{
      layout = layout,
      stateMap = TEST_WORLD.stateMap
  }
  asserts.EQ(tostring(grid), '22  \n' ..
                             '2222\n' ..
                             '222 \n')
end

function tests.createPiece()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece = grid:createPiece('type0', {pos = {2, 0}, orientation = 'E'})
  asserts.EQ(tostring(grid), '  0  \n')
  asserts.tablesEQ(grid:transform(piece), {pos = {2, 0}, orientation = 'E'})
end

function tests.userState()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {2, 0}, orientation = 'E'})
  local userState = {"Hello"}
  grid:setUserState(piece0, userState)
  local piece1 = grid:createPiece('type1', {pos = {0, 0}, orientation = 'E'})
  asserts.EQ(userState, grid:userState(piece0))
  asserts.EQ(nil, grid:userState(piece1))
end

function tests.clearUserState()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {2, 0}, orientation = 'E'})
  local userState = {"Hello"}
  grid:setUserState(piece0, userState)
  local piece1 = grid:createPiece('type1', {pos = {0, 0}, orientation = 'E'})
  asserts.EQ(userState, grid:userState(piece0))
  asserts.EQ(nil, grid:userState(piece1))
  grid:setUserState(piece0, nil)
  asserts.EQ(nil, grid:userState(piece0))
end

function tests.canRemovePiece()
  local random = require 'system.random'
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece = grid:createPiece('type0', {pos = {2, 0}, orientation = 'E'})
  asserts.EQ(tostring(grid), '  0  \n')
  grid:removePiece(piece)
  grid:update(random)
  asserts.EQ(tostring(grid), '     \n')
end

function tests.canRotatePiece()
  local random = require 'system.random'
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  grid:setOrientation(piece, 'E')
  grid:turn(piece, 1)
  grid:update(random)
  asserts.tablesEQ(grid:transform(piece), {pos = {0, 0}, orientation = 'S'})
end

function tests.canPushPiece()
  local random = require 'system.random'
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece = grid:createPiece('type0', {pos = {0, 0}, orientation = 'E'})
  grid:moveAbs(piece, 'E')
  grid:moveRel(piece, 'N')
  grid:update(random)
  asserts.EQ(tostring(grid), '  0  \n')
  asserts.tablesEQ(grid:transform(piece), {pos = {2, 0}, orientation = 'E'})
end

function tests.canPushPieceTorus()
  local random = require 'system.random'
  local width, height = 5, 6
  local grid = TEST_WORLD.world:createGrid{
      size = {width = width, height = height},
      topology = grid_world.TOPOLOGY.TORUS
  }
  local piece = grid:createPiece('type0', {pos = {0, 0}, orientation = 'E'})
  for i = 1, 10 do
    grid:moveAbs(piece, 'E')
    grid:update(random)
    local x = i % width
    asserts.tablesEQ(grid:position(piece), {x, 0})
  end
  for i = 9, -10, -1 do
    grid:moveAbs(piece, 'W')
    grid:update(random)
    local x = i % width
    asserts.tablesEQ(grid:position(piece), {x, 0})
  end
  asserts.tablesEQ(grid:position(piece), {0, 0})
  for j = 1, 10 do
    grid:moveAbs(piece, 'S')
    grid:update(random)
    local y = j % height
    asserts.tablesEQ(grid:position(piece), {0, y}, tostring(j))
  end
  for j = 9, -10, -1 do
    grid:moveAbs(piece, 'N')
    grid:update(random)
    local y = j % height
    asserts.tablesEQ(grid:position(piece), {0, y})
  end
end

function tests.canBlockPieceCallback()
  local random = require 'system.random'

  local callbacks = mock()
  when(callbacks).onBlocked(capture.dotDotDot())
  local grid = TEST_WORLD.world:createGrid{
      size = {width = 5, height = 1},
      stateCallbacks = {type0 = callbacks}
  }
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'E'})
  local piece1 = grid:createPiece('type0', {pos = {2, 0}, orientation = 'E'})
  asserts.EQ(tostring(grid), '0 0  \n')
  grid:moveAbs(piece0, 'E')
  grid:update(random)
  verifyNoMoreInteractions(callbacks)
  asserts.EQ(tostring(grid), ' 00  \n')
  -- Blocked by piece1:
  grid:moveAbs(piece0, 'E')
  grid:update(random)
  asserts.EQ(tostring(grid), ' 00  \n')
  verify(callbacks).onBlocked(grid, piece0, piece1)
  -- Blocked by out of bounds:
  grid:moveAbs(piece0, 'N')
  grid:update(random)
  verify(callbacks).onBlocked(grid, piece0, nil)
end

function tests.canPushPieceConnected()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type0', {pos = {1, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type0', {pos = {2, 0}, orientation = 'N'})
  grid:connect(piece0, piece1)
  grid:connect(piece1, piece2)
  grid:moveAbs(piece1, 'E')
  grid:update(random)
  asserts.EQ(tostring(grid), ' 000 \n')
  grid:disconnect(piece2)
  grid:moveAbs(piece1, 'W')
  grid:update(random)
  asserts.EQ(tostring(grid), '00 0 \n')
  grid:connect(piece0, piece2)
  grid:moveAbs(piece2, 'E')
  grid:update(random)
  asserts.EQ(tostring(grid), ' 00 0\n')
end

function tests.canPushPieceConnectedTorus()
  local grid = TEST_WORLD.world:createGrid{
      size = {width = 3, height = 1},
      topology = grid_world.TOPOLOGY.TORUS
  }
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type1', {pos = {1, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type2', {pos = {2, 0}, orientation = 'N'})
  asserts.EQ(tostring(grid), '0 2\n')
  grid:connect(piece0, piece1)
  grid:connect(piece1, piece2)
  grid:moveAbs(piece1, 'E')
  grid:update(random)
  asserts.EQ(tostring(grid), '20 \n')
  grid:moveAbs(piece1, 'W')
  grid:update(random)
  asserts.EQ(tostring(grid), '0 2\n')
  grid:moveAbs(piece2, 'W')
  grid:update(random)
  asserts.EQ(tostring(grid), ' 20\n')
end

function tests.canDisonnectAll()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type0', {pos = {1, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type0', {pos = {2, 0}, orientation = 'N'})
  grid:connect(piece0, piece1)
  grid:connect(piece1, piece2)
  grid:disconnectAll(piece0)
  grid:moveAbs(piece2, 'E')
  grid:update(random)
  asserts.EQ(tostring(grid), '00 0 \n')
  grid:moveAbs(piece1, 'E')
  grid:update(random)
  asserts.EQ(tostring(grid), '0 00 \n')
  grid:moveAbs(piece0, 'E')
  grid:update(random)
  asserts.EQ(tostring(grid), ' 000 \n')
end

function tests.canTeleportPiece()
  local grid = TEST_WORLD.world:createGrid{
      layout = '0    0',
      stateMap = TEST_WORLD.stateMap
  }
  local piece = grid:createPiece('type0', {pos = {1, 0}, orientation = 'E'})
  asserts.EQ(tostring(grid), '00   0\n')
  grid:teleport(piece, {4, 0})
  grid:update(random)
  asserts.EQ(tostring(grid), '0   00\n')
  grid:teleport(piece, {5, 0})
  grid:update(random)
  asserts.EQ(tostring(grid), '0   00\n')
end

function tests.setStatePiece()
  local grid = TEST_WORLD.world:createGrid{
      layout = '0    0',
      stateMap = TEST_WORLD.stateMap
  }
  local piece = grid:createPiece('type0', {pos = {1, 0}, orientation = 'E'})
  grid:update(random)
  asserts.EQ(tostring(grid), '00   0\n')
  grid:setState(piece, 'type1') -- Type 1 is invisible.
  grid:update(random)
  asserts.EQ(tostring(grid), '0    0\n')
  grid:setState(piece, 'type2')
  grid:update(random)
  asserts.EQ(tostring(grid), '02   0\n')
  grid:setState(piece, 'typeOffGrid')
  grid:update(random)
  asserts.EQ(tostring(grid), '0    0\n')
  grid:setState(piece, 'type2')
  grid:update(random)
  asserts.EQ(tostring(grid), '02   0\n')
  grid:setState(piece, 'type1')
  grid:setState(piece, 'type2')
  grid:update(random)
  asserts.EQ(tostring(grid), '02   0\n')
end

function tests.teleportToGroup()
  local world = grid_world.World{
      renderOrder = {'spawns', 'players'},
      types = {
          player0 = {
              layer = 'players',
              sprite = '0Player',
          },
          player1 = {
              layer = 'players',
              sprite = '1Player',
          },
          spawn = {
              layer = 'spawns',
              sprite = 'Spawns',
              groups = {'spawns'},
          },
          typeOffGrid = {}
      }
  }

  local grid = world:createGrid{
      layout = 'SSS',
      stateMap = {['S'] = 'spawn'}
  }

  local offGrid = {pos = {-1, -1}, orientation = 'N'}
  local p0 = grid:createPiece('player0', offGrid)
  local p1 = grid:createPiece('player0', offGrid)
  local p2 = grid:createPiece('player0', offGrid)
  local p3 = grid:createPiece('player0', offGrid)
  asserts.EQ(tostring(grid), 'SSS\n')
  grid:teleportToGroup(p0, 'spawns')
  grid:update(random)
  grid:teleportToGroup(p0, 'spawns')
  grid:update(random)
  grid:teleportToGroup(p1, 'spawns')
  grid:teleportToGroup(p2, 'spawns')
  grid:update(random)
  asserts.EQ(tostring(grid), '000\n')
  grid:teleportToGroup(p3, 'spawns')
  grid:update(random)
  grid:teleport(p0, offGrid.pos)
  grid:update(random)
  asserts.EQ(tostring(grid), '000\n')
  grid:teleport(p1, offGrid.pos)
  grid:teleport(p2, offGrid.pos)
  grid:teleport(p3, offGrid.pos)
  grid:update(random)
  asserts.EQ(tostring(grid), '000\n')
  grid:teleportToGroup(p0, 'spawns', 'player1')
  grid:teleportToGroup(p1, 'spawns', 'player1')
  grid:teleportToGroup(p2, 'spawns', 'player1')
  grid:update(random)
  asserts.EQ(tostring(grid), '111\n')
end

function tests.groupCountWorks()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type1', {pos = {1, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type2', {pos = {2, 0}, orientation = 'N'})
  local piece3 = grid:createPiece('type0', {pos = {3, 0}, orientation = 'N'})
  local piece4 = grid:createPiece('type0', {pos = {4, 0}, orientation = 'N'})
  asserts.EQ(grid:groupCount('type0'), 3)
  asserts.EQ(grid:groupCount('type1'), 1)
  asserts.EQ(grid:groupCount('type0or1'), 4)
  asserts.EQ(grid:groupCount('type2'), 1)
  asserts.EQ(grid:groupCount('all'), 5)
end

function tests.shuffledGroupWorks()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type1', {pos = {1, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type2', {pos = {2, 0}, orientation = 'N'})
  local piece3 = grid:createPiece('type0', {pos = {3, 0}, orientation = 'N'})
  local piece4 = grid:createPiece('type0', {pos = {4, 0}, orientation = 'N'})
  asserts.EQ(#grid:groupShuffled(random, 'type0'), 3)
  asserts.EQ(#grid:groupShuffled(random, 'type1'), 1)
  asserts.EQ(#grid:groupShuffled(random, 'type0or1'), 4)
  asserts.EQ(#grid:groupShuffled(random, 'type2'), 1)
  asserts.EQ(#grid:groupShuffled(random, 'all'), 5)

  asserts.EQ(#grid:groupShuffledWithCount(random, 'type0or1', 0), 0)
  asserts.EQ(#grid:groupShuffledWithCount(random, 'type0or1', 1), 1)
  asserts.EQ(#grid:groupShuffledWithCount(random, 'type0or1', 2), 2)
  asserts.EQ(#grid:groupShuffledWithCount(random, 'type0or1', 3), 3)
  asserts.EQ(#grid:groupShuffledWithCount(random, 'type0or1', 4), 4)
  asserts.EQ(#grid:groupShuffledWithCount(random, 'type0or1', 5), 4)

  for _, piece in ipairs(
    grid:groupShuffledWithProbability(random, 'type0or1', 0.5)) do
    assert(piece and piece ~= piece2)
  end

  local count = 0
  for i = 1, 100 do
    count = count + #grid:groupShuffledWithProbability(random, 'type0or1', 0.5)
  end
  asserts.LT(200 - 50, count)
  asserts.LT(count, 200 + 50)
end

function tests.canRayCast()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type0', {pos = {2, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type0', {pos = {4, 0}, orientation = 'N'})
  local result0 = {grid:rayCast('layer0', {0, 0}, {4, 0})}
  asserts.tablesEQ(result0, {true, piece1, {2, 0}})
  local result1 = {grid:rayCast('layer0', {2, 0}, {0, 0})}
  asserts.tablesEQ(result1, {true, piece0, {0, 0}})
  local result2 = {grid:rayCast('layer0', {0, 0}, {1, 0})}
  asserts.tablesEQ(result2, {false, nil, {1, 0}})
end

function tests.canRayCastDirection()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type0', {pos = {2, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type0', {pos = {4, 0}, orientation = 'N'})
  local result0 = {grid:rayCastDirection('layer0', {0, 0}, {4, 0})}
  asserts.tablesEQ(result0, {true, piece1, {2, 0}})
  local result1 = {grid:rayCastDirection('layer0', {2, 0}, {-2, 0})}
  asserts.tablesEQ(result1, {true, piece0, {-2, 0}})
  local result2 = {grid:rayCastDirection('layer0', {0, 0}, {1, 0})}
  asserts.tablesEQ(result2, {false, nil, {1, 0}})
end

function tests.groupRandomWorks()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  local piece1 = grid:createPiece('type1', {pos = {1, 0}, orientation = 'N'})
  local piece2 = grid:createPiece('type2', {pos = {2, 0}, orientation = 'N'})
  local piece3 = grid:createPiece('type0', {pos = {3, 0}, orientation = 'N'})
  local piece4 = grid:createPiece('type0', {pos = {4, 0}, orientation = 'N'})
  local type0 = {[piece0] = true, [piece3] = true, [piece4] = true}
  local type1 = {[piece1] = true}
  local type2 = {[piece2] = true}
  local type0Or1 = {
      [piece0] = true, [piece1] = true, [piece3] = true, [piece4] = true
  }

  local all = {
      [piece0] = true,
      [piece1] = true,
      [piece2] = true,
      [piece3] = true,
      [piece4] = true
  }
  for i = 1, 100 do
    assert(type0[grid:groupRandom(random, 'type0')])
    assert(type1[grid:groupRandom(random, 'type1')])
    assert(type0Or1[grid:groupRandom(random, 'type0or1')])
    assert(type2[grid:groupRandom(random, 'type2')])
    assert(all[grid:groupRandom(random, 'all')])
  end
end

function tests.transformWorks()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece = grid:createPiece('type0', {pos = {2, 0}, orientation = 'E'})
  asserts.tablesEQ(grid:transform(piece), {pos = {2, 0}, orientation = 'E'})
end

function tests.stateWorks()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece = grid:createPiece('type2', {pos = {2, 0}, orientation = 'E'})
  asserts.EQ(grid:state(piece), 'type2')
end

function tests.stateErrorOnInvalidPiece()
  local grid = TEST_WORLD.world:createGrid{
      layout = '0 1 2',
      stateMap = TEST_WORLD.stateMap
  }

  asserts.shouldFail(function() grid:state(nil) end, 'state')
end

function tests.updateAndFramesWork()
  local grid = TEST_WORLD.world:createGrid{size = {width = 5, height = 1}}
  local piece0 = grid:createPiece('type2', {pos = {0, 0}, orientation = 'E'})
  grid:update(random)
  local piece1 = grid:createPiece('type2', {pos = {1, 0}, orientation = 'E'})
  grid:update(random)
  local piece2 = grid:createPiece('type2', {pos = {2, 0}, orientation = 'E'})
  grid:update(random)
  local piece3 = grid:createPiece('type2', {pos = {3, 0}, orientation = 'E'})
  grid:update(random)
  local piece4 = grid:createPiece('type2', {pos = {4, 0}, orientation = 'E'})
  asserts.EQ(grid:frames(piece0), 4)
  asserts.EQ(grid:frames(piece1), 3)
  asserts.EQ(grid:frames(piece2), 2)
  asserts.EQ(grid:frames(piece3), 1)
  asserts.EQ(grid:frames(piece4), 0)
end

function tests.updateCallbackWorks()
  local framesPhase1 = {}
  local piecesPhase1 = {}
  local onUpdateMock = mock()
  when(onUpdateMock).funcPhase1(capture.dotDotDot())
  local grid = TEST_WORLD.world:createGrid{
      size = {width = 5, height = 1},
      stateCallbacks = {type2 = {onUpdate = onUpdateMock}},
  }
  grid:setUpdater{update = 'phase1', group = 'type2'}
  local piece0 = grid:createPiece('type2', {pos = {0, 0}, orientation = 'E'})
  grid:update(random)
  local piece1 = grid:createPiece('type2', {pos = {1, 0}, orientation = 'E'})
  grid:update(random)
  verify(onUpdateMock).funcPhase1(grid, 0, piece0)
  local updateFrame = capture.numberType()
  local piece = capture.numberType()
  verify(onUpdateMock).funcPhase1(grid, updateFrame, piece)
  verify(onUpdateMock).funcPhase1(grid, updateFrame, piece)
  asserts.NE(updateFrame.get(), updateFrame.get())
  asserts.NE(piece.get(), piece.get())
end

function tests.callBackOnEnterLeaveAddRemove()
  local act = ''

  local callbacks = mock()
  local contact = mock()
  callbacks.onContact = {contact2 = contact}
  when(callbacks).onAdd(capture.dotDotDot())
  when(callbacks).onRemove(capture.dotDotDot())
  when(contact).enter(capture.dotDotDot())
  when(contact).leave(capture.dotDotDot())
  local grid = TEST_WORLD.world:createGrid{
      size = {width = 5, height = 1},
      stateCallbacks = {type0 = callbacks},
  }
  local piece0 = grid:createPiece('type0', {pos = {0, 0}, orientation = 'N'})
  verify(callbacks).onAdd(grid, piece0)
  local piece1 = grid:createPiece('type2', {pos = {1, 0}, orientation = 'N'})
  grid:moveAbs(piece0, 'E')
  grid:update(random)
  verify(contact).enter(grid, piece0, piece1)
  grid:moveAbs(piece0, 'E')
  grid:update(random)
  verify(contact).leave(grid, piece0, piece1)
  grid:setState(piece0, 'type1')
  grid:update(random)
  verify(callbacks).onRemove(grid, piece0)
  verifyNoMoreInteractions(callbacks)
  verifyNoMoreInteractions(contact)
end

function tests.updateCallbackWorksWithProbability()
  local framesPhase1 = {}
  local grid = TEST_WORLD.world:createGrid{
      size = {width = 5, height = 1},
      stateCallbacks = {
          type2 = {
              onUpdate = {
                  phase1 = function(g, piece, frames)
                    framesPhase1[#framesPhase1 + 1] = frames
                  end
              }
          }
      },
  }
  random:seed(0)
  grid:setUpdater{update = 'phase1', group = 'type2', probability = 0.5}
  local piece0 = grid:createPiece('type2', {pos = {0, 0}, orientation = 'E'})
  for i = 0, 1000 do
    grid:update(random)
  end
  local mean = 0
  for i, frames in ipairs(framesPhase1) do
    mean = mean + frames / #framesPhase1
  end
  asserts.LT(480, #framesPhase1)
  asserts.LT(#framesPhase1, 520)
  asserts.LT(480, mean)
  asserts.LT(mean, 520)
end

function tests.updateCallbackWorksWithProbability()
  local grid = TEST_WORLD.world:createGrid{
      layout = ' ',
      stateMap = TEST_WORLD.stateMap,
  }

  asserts.shouldFail(
      function()
        grid:setUpdater{update = 'phase1', group = 'type2', probability = 0 / 0}
      end,
      '\'probability\' must be a number'
  )
end

function tests.updateCallbackWorksWithStartFrame()
  local framesPhase1 = {}
  local onUpdate = mock()
  when(onUpdate).funcPhase1(capture.dotDotDot())
  local grid = TEST_WORLD.world:createGrid{
      size = {width = 5, height = 1},
      stateCallbacks = {type2 = {onUpdate = onUpdate}},
  }
  grid:setUpdater{update = 'phase1', group = 'type2', startFrame = 5}
  local piece0 = grid:createPiece('type2', {pos = {0, 0}, orientation = 'E'})
  for i = 0, 6 do
    grid:update(random)
  end
  verify(onUpdate).funcPhase1(grid, piece0, 5)
  verify(onUpdate).funcPhase1(grid, piece0, 6)
  verifyNoMoreInteractions(onUpdate)
end

function tests.canCallHitBeam()
  local world = grid_world.World{
      renderOrder = {'dot', 'pieceLayer', 'hitLayer'},
      types = {
          player = {
              layer = 'pieceLayer',
              sprite = 'Player',
          },
          dot = {
              layer = 'dot',
              sprite = '.',
          }
      },
      hits = {
          hit = {
            layer = 'hitLayer',
            sprite = 'ohitSprite',
          },
      },
  }
  local layout = '\n' ..
      '..........\n' ..
      '..........\n' ..
      '..........\n' ..
      '..........\n' ..
      '..........\n'

  local playerCount = 0
  local dotCount = 0
  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot'},
      stateCallbacks = {
          player = {
              onHit = {
                  hit = function(piece, instigator)
                    playerCount = playerCount + 1
                    return true
                  end,
              }
          },
          dot = {
              onHit = {
                  hit = function(piece, instigator)
                    dotCount = dotCount + 1
                    return false
                  end,
              }
          },
      }
  }
  local piece0 = grid:createPiece('player', {pos = {0, 2}, orientation = 'E'})
  grid:hitBeam(piece0, 'hit', 8, 1)
  grid:update(random)
  local expected = '..........\n' ..
                   'oooooooo..\n' ..
                   'Poooooooo.\n' ..
                   'oooooooo..\n' ..
                   '..........\n'

  asserts.EQ(dotCount, 24)
  asserts.EQ(playerCount, 0)
  dotCount = 0
  asserts.EQ(tostring(grid), expected)
  grid:update(random)
  expected = '..........\n' ..
             '..........\n' ..
             'P.........\n' ..
             '..........\n' ..
             '..........\n'
  asserts.EQ(tostring(grid), expected)
  local piece1 = grid:createPiece('player', {pos = {3, 3}, orientation = 'N'})
  grid:hitBeam(piece0, 'hit', 8, 1)
  grid:update(random)
  asserts.EQ(dotCount, 20, tostring(dotCount))
  asserts.EQ(playerCount, 1)
  expected = '..........\n' ..
             'oooooooo..\n' ..
             'Poooooooo.\n' ..
             'oooP......\n' ..
             '..........\n'
  asserts.EQ(tostring(grid), expected)
end

function tests.canQueryPosition()
  local world = grid_world.World{
      renderOrder = {},
      types = {
          player = {
              layer = 'piece',
              sprite = 'Player',
          },
          dot = {
              layer = 'layer',
              sprite = '.',
          }
      }
  }
  local layout = '\n' ..
      '..........\n' ..
      '..........\n' ..
      '..........\n' ..
      '..........\n' ..
      '..........\n'

  local playerCount = 0
  local dotCount = 0
  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot'},
  }
  local piece0 = grid:createPiece('player', {pos = {2, 2}, orientation = 'E'})
  local dot22 = grid:queryPosition('layer', {2, 2})
  asserts.EQ(grid:state(dot22), 'dot')

  local piece22 = grid:queryPosition('piece', {2, 2})
  asserts.EQ(piece22, piece0)

  local piece44 = grid:queryPosition('piece', {4, 4})
  asserts.EQ(piece44, nil)
end

function tests.canQueryDisc()
  local world = grid_world.World{
      renderOrder = {},
      types = {
          player = {
              layer = 'piece',
              sprite = 'Player',
          },
          dot = {
              layer = 'layer',
              sprite = '.',
          },
          plus = {
              layer = 'layer',
              sprite = '+',
          }
      }
  }
  local layout = '\n' ..
      '.....+.....\n' ..
      '....++.....\n' ..
      '....+......\n' ..
      '...++......\n' ..
      '.+++.......\n' ..
      '+..........\n'

  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot', ['+'] = 'plus'},
  }
  local countDot = 0
  for piece, pos in pairs(grid:queryDisc('layer', {0, 0}, 4)) do
    local radiusSqr = pos[1] * pos[1] + pos[2] * pos[2]
    asserts.LE(radiusSqr, 4 * 4)
    asserts.EQ(grid:state(piece), 'dot')
    countDot = countDot + 1
  end
  asserts.EQ(countDot, 17)
  countDot = 0
  local countPlus = 0
  for piece, pos in pairs(grid:queryDisc('layer', {0, 0}, 5)) do
    local radiusSqr = pos[1] * pos[1] + pos[2] * pos[2]
    asserts.LE(radiusSqr, 5 * 5)
    local state = grid:state(piece)
    if state == 'dot' then
      countDot = countDot + 1
    elseif state == 'plus' then
      countPlus = countPlus + 1
    else
      error('Invalid type ' .. state)
    end
  end
  asserts.EQ(countDot, 17)
  asserts.EQ(countPlus, 9)
end

function tests.canQueryDiscTorus()
    local world = grid_world.World{
      renderOrder = {},
      types = {
          player = {
              layer = 'piece',
              sprite = 'Player',
          },
          dot = {
              layer = 'layer',
              sprite = '.',
          },
          plus = {
              layer = 'layer',
              sprite = '+',
          },
          star = {
              layer = 'layer',
              sprite = '*',
          }
      }
  }
  local layout = '\n' ..
      '*****+............+****\n' ..
      '****++............++***\n' ..
      '****+..............+***\n' ..
      '***++..............++**\n' ..
      '*+++................+++\n' ..
      '+......................\n' ..
      '.......................\n' ..
      '.......................\n' ..
      '+......................\n' ..
      '*+++................+++\n' ..
      '***++..............++**\n' ..
      '****+..............+***\n' ..
      '****++............++***\n'

  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot', ['+'] = 'plus', ['*'] = 'star'},
      topology = grid_world.TOPOLOGY.TORUS
  }
  local countStar = 0
  for piece, pos in pairs(grid:queryDisc('layer', {0, 0}, 4)) do
    local radiusSqr = pos[1] * pos[1] + pos[2] * pos[2]
    asserts.LE(radiusSqr, 4 * 4)
    asserts.EQ(grid:state(piece), 'star')
    countStar = countStar + 1
  end
  asserts.EQ(countStar, 49)
  countStar = 0
  local countPlus = 0
  for piece, pos in pairs(grid:queryDisc('layer', {0, 0}, 5)) do
    local radiusSqr = pos[1] * pos[1] + pos[2] * pos[2]
    asserts.LE(radiusSqr, 5 * 5)
    local state = grid:state(piece)
    if state == 'star' then
      countStar = countStar + 1
    elseif state == 'plus' then
      countPlus = countPlus + 1
    else
      error('Invalid type ' .. state)
    end
  end
  asserts.EQ(countStar, 49)
  asserts.EQ(countPlus, 32)
end


function tests.canQueryDiamond()
  local world = grid_world.World{
      renderOrder = {},
      types = {
          player = {
              layer = 'piece',
              sprite = 'Player',
          },
          dot = {
              layer = 'layer',
              sprite = '.',
          },
          plus = {
              layer = 'layer',
              sprite = '.',
          }
      }
  }
  local layout = '\n' ..
      '.....+.....\n' ..
      '....+......\n' ..
      '...+.......\n' ..
      '..+........\n' ..
      '.+.........\n' ..
      '+..........\n'

  local playerCount = 0
  local dotCount = 0
  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot', ['+'] = 'plus'},
  }
  local countDot = 0
  for piece, pos in pairs(grid:queryDiamond('layer', {0, 0}, 4)) do
    local length1 = pos[1] + pos[2]
    asserts.LE(length1, 4)
    asserts.EQ(grid:state(piece), 'dot')
    countDot = countDot + 1
  end
  asserts.EQ(countDot, 15)
  countDot = 0
  local countPlus = 0
  for piece, pos in pairs(grid:queryDiamond('layer', {0, 0}, 5)) do
    local length1 = pos[1] + pos[2]
    asserts.LE(length1, 5)
    local state = grid:state(piece)
    if state == 'dot' then
      countDot = countDot + 1
    elseif state == 'plus' then
      countPlus = countPlus + 1
    else
      error('Invalid type ' .. state)
    end
  end
  asserts.EQ(countDot, 15)
  asserts.EQ(countPlus, 6)
end

function tests.canQueryDiamondTorus()
  local world = grid_world.World{
      renderOrder = {},
      types = {
          player = {
              layer = 'piece',
              sprite = 'Player',
          },
          dot = {
              layer = 'layer',
              sprite = '.',
          },
          plus = {
              layer = 'layer',
              sprite = '.',
          },
          star = {
              layer = 'layer',
              sprite = '*',
          }
      }
  }
  local layout = '\n' ..
      '***+..+**\n' ..
      '**+....+*\n' ..
      '*+......+\n' ..
      '+........\n' ..
      '.........\n' ..
      '+........\n' ..
      '*+......+\n' ..
      '**+....+*\n'

  local playerCount = 0
  local dotCount = 0
  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot', ['+'] = 'plus', ['*'] = 'star'},
      topology = grid_world.TOPOLOGY.TORUS,
  }
  local countStar = 0
  for piece, pos in pairs(grid:queryDiamond('layer', {0, 0}, 2)) do
    local length1 = pos[1] + pos[2]
    asserts.LE(length1, 4)
    asserts.EQ(grid:state(piece), 'star')
    countStar = countStar + 1
  end
  asserts.EQ(countStar, 13)
  countStar = 0
  local countPlus = 0
  for piece, pos in pairs(grid:queryDiamond('layer', {0, 0}, 3)) do
    local length1 = pos[1] + pos[2]
    asserts.LE(length1, 5)
    local state = grid:state(piece)
    if state == 'star' then
      countStar = countStar + 1
    elseif state == 'plus' then
      countPlus = countPlus + 1
    else
      error('Invalid type ' .. state)
    end
  end
  asserts.EQ(countStar, 13)
  asserts.EQ(countPlus, 12)
end

function tests.canQueryRectangle()
  local world = grid_world.World{
      renderOrder = {},
      types = {
          player = {
              layer = 'piece',
              sprite = 'Player',
          },
          dot = {
              layer = 'layer',
              sprite = '.',
          },
          plus = {
              layer = 'layer',
              sprite = '.',
          }
      }
  }
  local layout = '\n' ..
      '...........\n' ..
      '.++++++....\n' ..
      '.+....+....\n' ..
      '.+....+....\n' ..
      '.+....+....\n' ..
      '.+....+....\n' ..
      '.++++++....\n' ..
      '...........\n'

  local playerCount = 0
  local dotCount = 0
  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot', ['+'] = 'plus'},
  }
  local countDot = 0
  for piece, pos in pairs(grid:queryRectangle('layer', {2, 2}, {5, 5})) do
    asserts.GE(pos[1], 2)
    asserts.LE(pos[1], 5)
    asserts.GE(pos[2], 2)
    asserts.LE(pos[2], 5)
    asserts.EQ(grid:state(piece), 'dot')
    countDot = countDot + 1
  end
  asserts.EQ(countDot, 4 * 4)
  countDot = 0
  local countPlus = 0
  for piece, pos in pairs(grid:queryRectangle('layer', {1, 1}, {6, 6})) do
    asserts.GE(pos[1], 1)
    asserts.LE(pos[1], 6)
    asserts.GE(pos[2], 1)
    asserts.LE(pos[2], 6)
    local state = grid:state(piece)
    if state == 'dot' then
      countDot = countDot + 1
    elseif state == 'plus' then
      countPlus = countPlus + 1
    else
      error('Invalid type ' .. state)
    end
  end
  asserts.EQ(countDot, 4 * 4)
  asserts.EQ(countPlus, 6 * 6 - 4 * 4)
end

function tests.canQueryRectangleTorus()
  local world = grid_world.World{
      renderOrder = {},
      types = {
          player = {
              layer = 'piece',
              sprite = 'Player',
          },
          dot = {
              layer = 'layer',
              sprite = '.',
          },
          plus = {
              layer = 'layer',
              sprite = '.',
          },
          star = {
              layer = 'layer',
              sprite = '*',
          }
      }
  }
  local layout = '\n' ..
      '...........\n' ..
      '++++.....++\n' ..
      '***+.....+*\n' ..
      '***+.....+*\n' ..
      '***+.....+*\n' ..
      '***+.....+*\n' ..
      '++++.....++\n' ..
      '...........\n'

  local playerCount = 0
  local dotCount = 0
  local grid = world:createGrid{
      layout = layout,
      stateMap = {['.'] = 'dot', ['+'] = 'plus', ['*'] = 'star'},
      topology = grid_world.TOPOLOGY.TORUS,
  }
  local countStar = 0
  for piece, pos in pairs(grid:queryRectangle('layer', {-1, 2}, {2, 5})) do
    asserts.EQ(grid:state(piece), 'star')
    countStar = countStar + 1
  end
  asserts.EQ(countStar, 4 * 4)
  countStar = 0
  local countPlus = 0
  for piece, pos in pairs(grid:queryRectangle('layer', {-2, 1}, {3, 6})) do
    local state = grid:state(piece)
    if state == 'star' then
      countStar = countStar + 1
    elseif state == 'plus' then
      countPlus = countPlus + 1
    else
      error('Invalid type ' .. state)
    end
  end
  asserts.EQ(countStar, 4 * 4)
  asserts.EQ(countPlus, 6 * 6 - 4 * 4)
end

return test_runner.run(tests)
