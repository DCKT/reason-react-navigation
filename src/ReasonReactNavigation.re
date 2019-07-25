module type RouterConfig = {
  type route;
  let routeFromUrl: ReasonReact.Router.url => route;
  let routeToUrl: route => string;
};

module CreateRouter = (Config: RouterConfig) => {
  let navigate = (route: Config.route) =>
    ReasonReact.Router.push(route->Config.routeToUrl);

  let replace = (route: Config.route) =>
    ReasonReact.Router.replace(route->Config.routeToUrl);

  let routerContext =
    React.createContext(
      ReasonReact.Router.dangerouslyGetInitialUrl() |> Config.routeFromUrl,
    );

  module RouterContextProvider = {
    let makeProps = (~value: Config.route, ~children, ()) => {
      "value": value,
      "children": children,
    };

    let make = React.Context.provider(routerContext);
  };

  module CurrentRouteProvider = {
    [@react.component]
    let make = (~children) => {
      let (currentRoute, setCurrentRoute) =
        React.useState(() =>
          ReasonReact.Router.dangerouslyGetInitialUrl() |> Config.routeFromUrl
        );

      React.useLayoutEffect1(
        () => {
          let watcherID =
            ReasonReact.Router.watchUrl(url =>
              setCurrentRoute(_ => url |> Config.routeFromUrl)
            );
          Some(() => ReasonReact.Router.unwatchUrl(watcherID));
        },
        [|setCurrentRoute|],
      );

      <RouterContextProvider value=currentRoute>
        children
      </RouterContextProvider>;
    };
  };

  module Link = {
    [@react.component]
    let make =
        (
          ~route: Config.route,
          ~className: option(string)=?,
          ~activeClassName: option(string)=?,
          ~onClick: option(ReactEvent.Mouse.t => unit)=?,
          ~children,
        ) => {
      let currentRoute = React.useContext(routerContext);
      let isCurrentRoute = currentRoute == route;

      <a
        href=route->Config.routeToUrl
        className={
          String.concat(
            " ",
            [
              className->Belt.Option.getWithDefault(""),
              isCurrentRoute ?
                activeClassName->Belt.Option.getWithDefault("") : "",
            ],
          )
        }
        onClick={
          e => {
            e->ReactEvent.Synthetic.preventDefault;
            switch (onClick) {
            | None => ()
            | Some(fn) => fn(e)
            };
            navigate(route);
          }
        }>
        children
      </a>;
    };
  };
};