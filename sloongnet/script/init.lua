Init = function( path )
    package.path = package.path .. ';' .. path .. 'script/?.lua';
    print(package.path);

    require('comm')
    require('epollproc')

end

