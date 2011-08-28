
(function ($) {
    $.extend({
        postJSON: function (url, data, callback, type) {
            this.ajax({
                type: "POST",
                url: url,
                contentType: "application/json; charset=utf-8",
                data: data ? JSON.stringify(data) : null,
                success: callback,
                dataType: type
            });
        }, JSON: function (method, url, data, callback, type) {
            this.ajax({
                type: method,
                url: url,
                contentType: "application/json; charset=utf-8",
                data: data ? JSON.stringify(data) : null,
                success: callback,
                dataType: type
            });
        }
    });
})(jQuery);