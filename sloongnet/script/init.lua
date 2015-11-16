Init = function( path )
    package.path = package.path .. ';' .. path .. 'script/?.lua';
    print(package.path);

    require('comm')
    require('epollproc')

end

test = function( pack )
    pack:setdata("test","testone");
    print(pack:getdata("test"));
    --print(pack)
end
