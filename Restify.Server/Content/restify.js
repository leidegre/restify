
(function ($) {
    $.extend({
        postJSON: function (url, data, callback, type) {
            this.ajax({
                type: "POST",
                url: "/restify/session/login",
                contentType: "application/json; charset=utf-8",
                data: JSON.stringify(data),
                success: callback,
                dataType: type
            });
        }
    });
})(jQuery);