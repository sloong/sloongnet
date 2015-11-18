Init = function( path )
    package.path = package.path .. ';' .. path .. 'script/?.lua';

    require('comm')
    require('epollproc')

end
